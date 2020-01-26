#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define TYPE_CTRL 1
#define TYPE_INT  2
#define TYPE_BULK 3
#define TYPE_ISOC 4

#define SIGN_EQ  1
#define SIGN_LT  2
#define SIGN_LTE 3
#define SIGN_GT  4
#define SIGN_GTE 5

#define VAR_ID    1
#define VAR_TYPE  2
#define VAR_SIZE  3
#define VAR_INDEX 4
#define VAR_EP    5
#define VAR_ERRS  6
#define N_VARS    6

#define XFER_TIMEOUT 1000

#define BUCKET_SIZE 0x30000

#define ISOC_RING_LEN 10

#define CMD_BUF_SIZE 1024
#define MAX_ARGS       32

#define EXIT 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef int (*Data_Xfer)(struct libusb_device_handle*, u8, u8*, int, int*, u32);
typedef void* (*Allocator)(int);

static struct libusb_device_handle *dev;

struct xfer_entry_t {
	int id;

	u8 type;
	u8 endpoint;
	u8 req_type;
	u8 req;

	u16 value;
	u16 index;

	int pkt_sz;
	int n_pkts;
	int frame;
	int n_errors;

	int res;

	int size;
	u8 *data;

	struct xfer_entry_t *next;
};
typedef struct xfer_entry_t Xfer_Entry;

typedef struct {
	int var;
	int min;
	int max;
} Filter;

struct isoc_ring_t {
	struct libusb_transfer *usb;
	struct isoc_ring_t *next;
};
typedef struct isoc_ring_t Isoc_Ring;

struct bucket_t {
	struct bucket_t *next;
	int idx;
	int used;
};
typedef struct bucket_t Bucket;

Bucket *first_bucket = NULL;
Bucket *head = NULL; // ;)
int n_buckets = 0;

void *allocate(int size) {
	if (size <= 0 || size > BUCKET_SIZE)
		return NULL;

	if (!head || (head && size > BUCKET_SIZE - head->used)) {
		Bucket **neck = head != NULL ? &head->next : &head;
		*neck = malloc(sizeof(Bucket) + BUCKET_SIZE);
		head = *neck;

		if (!first_bucket)
			first_bucket = head;

		head->next = NULL;
		head->idx = n_buckets++;
		head->used = 0;
	}

	void *ptr = (void*)(&head[1]) + head->used;
	head->used += size;

	return ptr;
}

void destroy_buckets(void) {
	head = first_bucket;
	if (!head)
		return;

	while (head) {
		Bucket *temp = head->next;
		free(head);
		head = temp;
	}

	first_bucket = head = NULL;
}

Xfer_Entry *first_xfer = NULL;
Xfer_Entry *head_xfer = NULL;

Xfer_Entry *new_transfer(void) {
	int id = -1;
	Xfer_Entry **neck = &head_xfer;

	if (head_xfer) {
		id = head_xfer->id;
		neck = &head_xfer->next;
	}

	*neck = allocate(sizeof(Xfer_Entry));
	head_xfer = *neck;
	memset(head_xfer, 0, sizeof(Xfer_Entry));

	if (!first_xfer)
		first_xfer = head_xfer;

	head_xfer->id = id + 1;
	return head_xfer;
}

int read_file(char *name, u8 **ptr, int max_size, Allocator ator) {
	FILE *f = fopen(name, "rb");
	if (!f)
		return -1;

	fseek(f, 0, SEEK_END);
	int sz = ftell(f);
	rewind(f);

	if (max_size > 0 && sz > max_size)
		sz = max_size;

	if (ator)
		*ptr = ator(sz);

	fread(*ptr, 1, sz, f);
	fclose(f);

	return sz;
}

int parse_byte_array(int n_args, char **args, int arg_idx, u8 **ptr, int max_size, Allocator ator) {
	char buf[1024] = {0};
	char *p = &buf[0];

	for (int i = arg_idx; i < n_args && p - buf < 1024; i++) {
		*p++ = '0';
		int n_digits = 0;

		for (int j = 0; j < strlen(args[i]) && p - buf < 1024; j++) {
			char c = args[i][j];
			if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
				*p++ = c;
				n_digits++;
			}
		}

		if (n_digits % 2 == 0) {
			memmove(p - (n_digits + 1), p - n_digits, n_digits);
			p--;
			*p = 0;
		}
	}

	int sz = strlen(buf) / 2;
	if (max_size > 0 && sz > max_size)
		sz = max_size;

	if (ator)
		*ptr = ator(sz);

	p = &buf[0];
	for (int i = 0; i < sz; i++) {
		int digit = 0;
		for (int j = 0; j < 2; j++) {
			digit <<= 4;
			if (*p >= '0' && *p <= '9')
				digit |= *p - '0';
			else if (*p >= 'A' && *p <= 'F')
				digit |= *p - 'A' + 0xa;
			else if (*p >= 'a' && *p <= 'f')
				digit |= *p - 'a' + 0xa;

			p++;
		}

		(*ptr)[i] = (u8)digit;
	}

	return sz;
}

int get_filter_part(Filter *flt, char *str, char **end) {
	char *p = str;
	while (*p && *p != '=' && *p != '<' && *p != '>')
		p++;

	if (*p == 0)
		return 0;

	int sign = -1;
	if (*p == '=') {
		sign = SIGN_EQ;
	}
	else if (*p == '<') {
		if (p[1] == '=') {
			sign = SIGN_LTE;
			p++;
		}
		else
			sign = SIGN_LT;
	}
	else if (*p == '>') {
		if (p[1] == '=') {
			sign = SIGN_GTE;
			p++;
		}
		else
			sign = SIGN_GT;
	}

	if (sign < 0) {
		printf("Unrecognised sign from \"%s\"\n", p);
		return sign;
	}

	p++;
	int n = strtol(p, &p, 0);
	*end = p;

	switch (sign) {
		case SIGN_EQ:
			flt->min = flt->max = n;
			break;
		case SIGN_GT:
			flt->min = n + 1;
			break;
		case SIGN_GTE:
			flt->min = n;
			break;
		case SIGN_LT:
			flt->max = n - 1;
			break;
		case SIGN_LTE:
			flt->max = n;
			break;
	}

	return sign;
}

Filter parse_filter(char *flt_str) {
	Filter flt = {-1, -1, -1};

	char *p = flt_str;
	while (*p >= 'a' && *p <= 'z')
		p++;

	int len = p - flt_str;
	if (!strncmp(flt_str, "id", len))
		flt.var = VAR_ID;
	else if (!strncmp(flt_str, "type", len))
		flt.var = VAR_TYPE;
	else if (!strncmp(flt_str, "size", len))
		flt.var = VAR_SIZE;
	else if (!strncmp(flt_str, "index", len))
		flt.var = VAR_INDEX;
	else if (!strncmp(flt_str, "ep", len))
		flt.var = VAR_EP;
	else if (!strncmp(flt_str, "errors", len))
		flt.var = VAR_ERRS;
	else {
		*p = 0;
		printf("Unrecognised variable \"%s\"\n", flt_str);
		return flt;
	}

	if (flt.var == VAR_TYPE) {
		while (*p && (*p < 'a' || *p > 'z'))
			p++;

		if (!strcmp(p, "ctrl"))
			flt.min = flt.max = TYPE_CTRL;
		else if (!strcmp(p, "int"))
			flt.min = flt.max = TYPE_INT;
		else if (!strcmp(p, "bulk"))
			flt.min = flt.max = TYPE_BULK;
		else if (!strcmp(p, "isoc"))
			flt.min = flt.max = TYPE_ISOC;
		else {
			flt.var = -1;
			printf("Unrecognised value \"%s\" for variable \"type\"\n", p);
		}

		return flt;
	}

	int sign_1 = get_filter_part(&flt, p, &p);
	if (sign_1 <= 0) {
		if (sign_1 == 0) {
			flt.var = -1;
			printf("No qualifier in \"%s\" was found\n", flt_str);
		}
		return flt;
	}

	int sign_2 = get_filter_part(&flt, p, &p);
	if (sign_2 <= 0)
		return flt;

	// At this point, we know that this filter specifies a range
	if (sign_1 == SIGN_EQ || sign_2 == SIGN_EQ) {
		flt.var = -1;
		printf("Invalid range spec in \"%s\" (found equals sign)\n", flt_str);
	}

	return flt;
}

int test_filters(Xfer_Entry *xfer, Filter *filters, int n_flts) {
	int i;
	Filter *flt = &filters[0];

	for (int i = 0; i < n_flts; i++, flt++) {
		int n;
		int spec_type = -1;

		switch (flt->var) {
			case VAR_ID:
				n = xfer->id;
				break;
			case VAR_TYPE:
				n = xfer->type;
				break;
			case VAR_SIZE:
				n = xfer->size;
				break;
			case VAR_INDEX:
				n = xfer->index;
				spec_type = TYPE_CTRL;
				break;
			case VAR_EP:
				n = (xfer->endpoint | xfer->req_type) & 0x1f;
				break;
			case VAR_ERRS:
				n = xfer->n_errors;
				spec_type = TYPE_ISOC;
				break;
			default:
				return 0;
		}

		int no_min = flt->min < 0;
		int no_max = flt->max < 0;
		int pass = (n >= flt->min || no_min) && (n <= flt->max || no_max);

		if (!pass || (xfer->type != spec_type && spec_type > 0))
			return 0;
	}

	return 1;
}

int help(int argc, char **args) {
	printf(
		"Commands:\n"
		"  exit / quit\n"
		"    Exits the program\n"
		"  help\n"
		"    Prints this message\n"
		"  select <interface> <alt setting>\n"
		"    Claim an interface and select an alt setting within that interface\n"
		"  ctrl <req type> <request> <value> <index> <length / file / byte array>\n"
		"    Issues a control transfer\n"
		"  int <endpoint> <length / file / byte array>\n"
		"    Issues an interrupt transfer\n"
		"  bulk <endpoint> <length / file / byte array>\n"
		"    Issues a bulk transfer\n"
		"  isoc <endpoint> <packet count> <packet size> [input file / byte array]\n"
		"    Issues an isochronous transfer\n"
		"  list [filter(s)...]\n"
		"    List all transfers that match certain criteria\n"
		"    See \"Filters\" for more info\n"
		"  save <file> [filter(s)...]\n"
		"    Saves packet data from all transfers that match certain criteria\n"
		"  exec <script file>\n"
		"    Loads a text file and interprets each line as a command\n\n"
		"Filters:\n"
		"  A way of selecting which transfers to view or save.\n"
		"  The format is: <variable> <sign> <value> [<sign> <value>]\n"
		"   eg. \"id > 10\"\n"
		"       id>10\n"
		"       \"id >= 7 <= 10\"\n"
		"       id>=7<=10\n"
		"       \"id = 0x2a\"\n"
		"       \"type = isoc\"\n"
		"  Note:\n"
		"   1) a filter must fit into a single argument,\n"
		"       ie. you must use quotes or avoid spaces\n"
		"   2) for string variables (eg. type), only the equals sign is allowed\n\n"
		"Filter Variables:\n"
		"  id\n"
		"    Select transfer by its id/index\n"
		"  type\n"
		"    Select for transfers of a certain type (out of ctrl, int, bulk, isoc)\n"
		"  size\n"
		"    Select for the size of a transfer's data\n"
		"  index\n"
		"    For control transfers only, select for the index parameter\n"
		"  ep\n"
		"    Select for the endpoint (or req type if ctrl) AND 0x1f\n"
		"  errors\n"
		"    For isochronous transfers only, select for the number of errors\n\n"
	);

	return 0;
}

int prev_iface = -1;

int sel(int argc, char **args) {
	int iface = strtol(args[1], NULL, 0);
	int alt   = strtol(args[2], NULL, 0);

	if (prev_iface >= 0 && iface != prev_iface)
		libusb_release_interface(dev, prev_iface);

	int res = libusb_claim_interface(dev, iface);
	if (res != 0) {
		printf("Failed to claim interface %d: %s (%d)\n\n", iface, libusb_strerror(res), res);
		return -1;
	}

	prev_iface = iface;

	res = libusb_set_interface_alt_setting(dev, iface, alt);
	if (res != 0) {
		printf("Failed to set alt setting %d on interface %d: %s (%d)\n\n", alt, iface, libusb_strerror(res), res);
		return -2;
	}

	return 0;
}

int ctrl(int argc, char **args) {
	Xfer_Entry *xfer = new_transfer();
	xfer->type = TYPE_CTRL;

	xfer->req_type = strtol(args[1], NULL, 16);
	xfer->req = strtol(args[2], NULL, 0);
	xfer->value = strtol(args[3], NULL, 0);
	xfer->index = strtol(args[4], NULL, 0);

	if (xfer->req_type & 0x80) {
		xfer->size = strtol(args[5], NULL, 0);
		xfer->data = allocate(xfer->size);
	}
	else {
		int sz = read_file(args[5], &xfer->data, 0, allocate);
		if (sz <= 0)
			sz = parse_byte_array(argc, args, 5, &xfer->data, 0, allocate);

		xfer->size = sz;
	}

	xfer->res = libusb_control_transfer(dev, xfer->req_type, xfer->req, xfer->value, xfer->index, xfer->data, xfer->size, XFER_TIMEOUT);	
	return 0;
}

Xfer_Entry *data_transfer(int argc, char **args, Data_Xfer func) {
	Xfer_Entry *xfer = new_transfer();
	xfer->endpoint = (u8)strtol(args[1], NULL, 16);

	if (xfer->endpoint & 0x80) {
		xfer->size = strtol(args[2], NULL, 0);
		xfer->data = allocate(xfer->size);
	}
	else {
		int sz = read_file(args[2], &xfer->data, 0, allocate);
		if (sz <= 0)
			sz = parse_byte_array(argc, args, 5, &xfer->data, 0, allocate);

		xfer->size = sz;
	}

	func(dev, xfer->endpoint, xfer->data, xfer->size, &xfer->res, XFER_TIMEOUT);
	return xfer;
}

int int_cmd(int argc, char **args) {
	Xfer_Entry *xfer = data_transfer(argc, args, libusb_interrupt_transfer);
	xfer->type = TYPE_INT;
	return 0;
}
int bulk(int argc, char **args) {
	Xfer_Entry *xfer = data_transfer(argc, args, libusb_bulk_transfer);
	xfer->type = TYPE_BULK;
	return 0;
}

Isoc_Ring isoc_chain[ISOC_RING_LEN] = {NULL};
Isoc_Ring *head_isoc = NULL;

void isoc_cb(struct libusb_transfer *usb_xfer) {
	int total = usb_xfer->num_iso_packets;
	int size = 0, errors = 0;
	struct libusb_iso_packet_descriptor *pk = &usb_xfer->iso_packet_desc[0];

	for (int i = 0; i < total; i++, pk++) {
		size += pk->actual_length;
		if (pk->status != 0)
			errors++;
	}

	Xfer_Entry *xfer = usb_xfer->user_data;
	xfer->size = size;
	xfer->res = xfer->size;
	xfer->n_errors = errors;

	//printf("isoc_cb() : Success rate = %d / %d\n", n, total);
}

int isoc(int argc, char **args) {
	Xfer_Entry *xfer = new_transfer();
	xfer->type = TYPE_ISOC;

	xfer->endpoint = strtol(args[1], NULL, 16);
	xfer->n_pkts   = strtol(args[2], NULL, 0);
	xfer->pkt_sz   = strtol(args[3], NULL, 0);

	xfer->size = xfer->n_pkts * xfer->pkt_sz;
	xfer->data = allocate(xfer->size);

	for (int i = 0; i < ISOC_RING_LEN; i++) {
		int needs_realloc = isoc_chain[i].usb && isoc_chain[i].usb->num_iso_packets != xfer->n_pkts;
		if (!isoc_chain[i].usb || needs_realloc) {
			if (needs_realloc)
				libusb_free_transfer(isoc_chain[i].usb);

			isoc_chain[i].usb = libusb_alloc_transfer(xfer->n_pkts);

			isoc_chain[i].next = i == ISOC_RING_LEN-1 ? &isoc_chain[0] : &isoc_chain[i+1];
		}
	}
	if (!head_isoc)
		head_isoc = &isoc_chain[0];

	if ((xfer->endpoint & 0x80) == 0) {
		if (argc < 5) {
			printf("Input file or array required for sending data w/ isochronous transfer\n");
			return -1;
		}

		int sz = read_file(args[4], &xfer->data, xfer->size, NULL);
		if (sz <= 0)
			sz = parse_byte_array(argc, args, 4, &xfer->data, xfer->size, NULL);

		if (sz < xfer->size)
			memset(xfer->data + sz, 0, xfer->size - sz);
	}

	libusb_fill_iso_transfer(head_isoc->usb, dev, xfer->endpoint, xfer->data, xfer->size, xfer->n_pkts, isoc_cb, xfer, XFER_TIMEOUT);
	libusb_set_iso_packet_lengths(head_isoc->usb, xfer->pkt_sz);

	int res = libusb_submit_transfer(head_isoc->usb);
	head_isoc = head_isoc->next;

	if (res >= 0) {
		res = libusb_handle_events(NULL);
		if (res < 0)
			printf("libusb_handle_events() = %d : %s\n\n", res, libusb_strerror(res));
	}
	else
		printf("libusb_submit_transfer() = %d : %s\n\n", res, libusb_strerror(res));

	return res < 0 ? res : 0;
}

int list(int argc, char **args) {
	Filter filters[N_VARS];
	int n_flts = 0;

	for (int i = 0; i < argc-1 && i < N_VARS; i++) {
		filters[i] = parse_filter(args[i+1]);
		if (filters[i].var < 0)
			return -1;

		n_flts++;
		for (int j = 0; j < n_flts-1; j++) {
			if (filters[i].var == filters[j].var) {
				printf("Cannot apply two filters on the same variable\n");
				return -2;
			}
		}
	}

	const char *types[] = {
		"control", "interrupt", "bulk", "isochronous"
	};

	Xfer_Entry *xfer = first_xfer;
	int count = 0;
	while (xfer) {
		count++;

		if (test_filters(xfer, filters, n_flts) == 0) {
			xfer = xfer->next;
			continue;
		}

		printf("%5d - type = %s, ", xfer->id, types[xfer->type - 1]);

		int i;
		switch (xfer->type) {
			case TYPE_CTRL:
				printf(
					"req type = %#x, req = %#x, value = %#x, index = %#x, data = ",
					xfer->req_type, xfer->req, xfer->value, xfer->index
				);
				for (i = 0; i < xfer->size; i++)
					printf("%02x ", xfer->data[i]);

				break;
			case TYPE_INT:
			case TYPE_BULK:
				printf("endpoint = %#x, data = ", xfer->endpoint);
				for (i = 0; i < xfer->size; i++)
					printf("%02x ", xfer->data[i]);

				break;
			case TYPE_ISOC:
				printf(
					"endpoint = %#x, max size = %d, actual size = %d, packets = %d, errors = %d",
					xfer->endpoint, xfer->n_pkts * xfer->pkt_sz, xfer->size, xfer->n_pkts, xfer->n_errors
				);
				break;
		}

		putchar('\n');
		xfer = xfer->next;
	}

	putchar('\n');
}

int save(int argc, char **args) {
	Filter filters[N_VARS];
	int n_flts = 0;

	for (int i = 0; i < argc-2 && i < N_VARS; i++) {
		filters[i] = parse_filter(args[i+2]);
		if (filters[i].var < 0)
			return -1;

		n_flts++;
		for (int j = 0; j < n_flts-1; j++) {
			if (filters[i].var == filters[j].var) {
				printf("Cannot apply two filters on the same variable\n");
				return -2;
			}
		}
	}

	FILE *f = NULL;
	Xfer_Entry *xfer = first_xfer;

	while (xfer) {
		if (test_filters(xfer, &filters[0], n_flts) == 0) {
			xfer = xfer->next;
			continue;
		}

		if (!f) {
			f = fopen(args[1], "wb");
			if (!f) {
				printf("Could not open file \"%s\"\n", args[1]);
				return -3;
			}
		}

		fwrite(xfer->data, 1, xfer->size, f);

		xfer = xfer->next;
	}
	if (f)
		fclose(f);

	return 0;
}

static char cmd_buf[CMD_BUF_SIZE];

int parse_and_run_command(char **args, const int max_args);

int exec(int argc, char **args) {
	FILE *f = fopen(args[1], "r");
	if (!f) {
		printf("Could not open script file %s\n", args[1]);
		return -1;
	}

	while (fgets(cmd_buf, CMD_BUF_SIZE, f)) {
		int res = parse_and_run_command(args, MAX_ARGS);
		if (res == EXIT)
			return res;
	}

	return 0;
}

struct Command {
	int (*func)(int, char **);
	const char *name;
	int min_args;
} cmd_table[] = {
	{help, "help", 1},
	{sel, "select", 3},
	{ctrl, "ctrl", 6},
	{int_cmd, "int", 3},
	{bulk, "bulk", 3},
	{isoc, "isoc", 4},
	{list, "list", 1},
	{save, "save", 2},
	{exec, "exec", 2}
};
const int N_CMDS = sizeof(cmd_table) / sizeof(struct Command);

int parse_and_run_command(char **args, const int max_args) {
	int n_args = 0;
	int quotes = 0;
	char *p = cmd_buf;
	for (int i = 0; i < CMD_BUF_SIZE; i++) {
		quotes ^= *p == '\"';
		int end = *p == '\n' || *p == '\r';

		if (*p == '\t')
			*p = ' ';

		if (end || (*p == ' ' && !quotes)) {
			*p = 0;
			n_args++;
			if (end) break;
		}
		p++;
	}

	if (!strlen(cmd_buf))
		return 0;

	if (n_args >= MAX_ARGS) {
		printf(
			"Command exceeds argument limit of %d.\n"
			"If you need to pass in a long sequence of bytes,\n"
			" try placing quotes around the sequence, eg. \"81 00 51 01\"\n\n",
			MAX_ARGS-1
		);
		return 0;
	}

	p = cmd_buf;
	for (int i = 0; i < n_args; i++) {
		args[i] = p;
		p += strlen(p) + 1;
	}

	if (!strcmp(args[0], "exit") || !strcmp(args[0], "quit"))
		return EXIT;

	int idx = -1;
	for (int i = 0; i < N_CMDS; i++) {
		if (!strcmp(args[0], cmd_table[i].name)) {
			idx = i;
			break;
		}
	}

	if (idx < 0) {
		printf("Unrecognised command \"%s\"\n\n", args[0]);
		return 0;
	}

	struct Command *cmd = &cmd_table[idx];
	if (n_args < cmd->min_args) {
		printf(
			"The %s command requires at least %d parameter(s).\n"
			"For more information, check out the help command.\n\n",
			cmd->name, cmd->min_args-1
		);
		return 0;
	}

	if (!cmd->func) {
		printf("The %s command has not yet been implemented\n\n", cmd->name);
		return 0;
	}

	return cmd->func(n_args, args);
}

int main(int argc, char **argv) {
	printf("USB Packet Shell\n\n");
	if (argc != 3) {
		printf(
			"Usage: %s <Vendor ID> <Product ID>\n",
			argv[0]
		);
		return 1;
	}

	if (libusb_init(NULL) < 0) {
		printf("Could not open libusb\n");
		return 2;
	}

	int vendor  = strtol(argv[1], NULL, 16);
	int product = strtol(argv[2], NULL, 16);

	dev = libusb_open_device_with_vid_pid(NULL, vendor, product);
	if (!dev) {
		printf("Could not open USB device %04x:%04x\n", vendor, product);
		libusb_exit(NULL);
		return 3;
	}

	printf("Opened device %04x:%04x\nType \"help\" for a list of recognised commands\n\n", vendor, product);

	char *args[MAX_ARGS];

	while (1) {
		printf("> ");
		fgets(cmd_buf, CMD_BUF_SIZE, stdin);
		putchar('\n');

		int res = parse_and_run_command(args, MAX_ARGS);
		if (res == EXIT)
			break;
	}

	if (head_isoc) {
		for (int i = 0; i < ISOC_RING_LEN; i++) {
			if (isoc_chain[i].usb)
				libusb_free_transfer(isoc_chain[i].usb);
		}
	}

	if (prev_iface >= 0)
		libusb_release_interface(dev, prev_iface);

	destroy_buckets();

	libusb_close(dev);
	libusb_exit(NULL);

	return 0;
}
