// All register info is straight from the Linux source (drivers/media/usb/em28xx/em28xx-reg.h)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define URB_ISOC 0
#define URB_INTR 1
#define URB_CTRL 2
#define URB_BULK 3

#define SET_INTERFACE 0xb

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

struct Register {
	int index;
	const char *str;
}
em28xx_regs[] = {
	{  0, "CHIP_CFG"},
	{  1, "CHIP_CFG2"},
	{  4, "GPOUT"},
	{  6, "I2C_CLK"},
	{  8, "AUDIO_SRC/GPIO_CTRL"},
	{  9, "GPIO_STATE"},
	{ 10, "CHIP_ID"},
	{ 12, "USB_SUSP"},
	{ 14, "AUDIO_SRC"},
	{ 15, "X_CLK"},
	{ 16, "V_IN_MODE"},
	{ 17, "V_IN_CTRL"},
	{ 18, "V_IN_ENABLE"},
	{ 20, "GAMMA"},
	{ 21, "R_GAIN"},
	{ 22, "G_GAIN"},
	{ 23, "B_GAIN"},
	{ 24, "R_OFFSET"},
	{ 25, "G_OFFSET"},
	{ 26, "B_OFFSET"},
	{ 27, "OVERFLOW"},
	{ 28, "HORI_START"},
	{ 29, "VERT_START"},
	{ 30, "C_WIDTH"},
	{ 31, "C_HEIGHT"},
	{ 32, "CONTRAST"}, // Y Gain
	{ 33, "BRIGHTNESS"}, // Y Offset
	{ 34, "SATURATION"}, // UV Gain
	{ 35, "B_BALANCE"}, // U Offset
	{ 36, "R_BALANCE"}, // V Offset
	{ 37, "SHARPNESS"},
	{ 38, "COMPRESSION"},
	{ 39, "OUT_FORMAT"},
	{ 40, "X_MIN"},
	{ 41, "X_MAX"},
	{ 42, "Y_MIN"},
	{ 43, "Y_MAX"},
	{ 48, "H_SCALE_LOW"},
	{ 49, "H_SCALE_HIGH"},
	{ 50, "V_SCALE_LOW"},
	{ 51, "V_SCALE_HIGH"},
	{ 52, "VBI_START_H"},
	{ 53, "VBI_START_V"},
	{ 54, "VBI_WIDTH"},
	{ 55, "VBI_HEIGHT"},
	{ 62, "EXT_MODEM_CTRL"},
	{ 64, "AC97_LSB"},
	{ 65, "AC97_MSB"},
	{ 66, "AC97_ADDR"},
	{ 67, "AC97_BUSY"},
	{ 69, "IR"},
	{ 76, "GPIO_CONFIG"},
	{ 78, "GPIO_POLARITY"},
	{ 80, "IR_CFG/GPIO_STICKY"},
	{ 81, "IR_81"},
	{ 82, "GPIO_MASK"},
	{ 84, "GPIO_STATUS"},
	{ 93, "TS1_PKT_SIZE"},
	{ 94, "TS2_PKT_SIZE"},
	{ 95, "TS_ENABLE"},
	{106, "SPDIF_OUT_SEL"},
	{114, "ANTIPOP"},
	{116, "EAPD_GPIO_ACCESS"},
	{128, "GPIO_P0_CTRL"},
	{129, "GPIO_P1_CTRL"},
	{130, "GPIO_P2_CTRL"},
	{131, "GPIO_P3_CTRL"},
	{132, "GPIO_P0_STATE"},
	{133, "GPIO_P1_STATE"},
	{134, "GPIO_P2_STATE"},
	{135, "GPIO_P3_STATE"}
};
const int N_REGS = sizeof(em28xx_regs) / sizeof(struct Register);

struct Reg_Value {
	int reg;
	int mask;
	int value;
	const char *str;
}
em28xx_reg_values[] = {
	{ 0, 0x80, 0x80, "VENDOR_AUDIO"},
	{ 0, 0x40, 0x40, "I2S_VOL_CAPABLE"},
	{ 0, 0x30, 0x30, "I2S_3/5_SAMP_RATES"},
	{ 0, 0x30, 0x20, "I2S_1/3_SAMP_RATES"},
	{ 0, 0x30, 0x10, "AC97"},
	{ 1, 0x10, 0x10, "TS_PRESENT"},
	{ 1, 0x0c, 0x00, "TS_REQ_INTERVAL_1MF"},
	{ 1, 0x0c, 0x04, "TS_REQ_INTERVAL_2MF"},
	{ 1, 0x0c, 0x08, "TS_REQ_INTERVAL_4MF"},
	{ 1, 0x0c, 0x0c, "TS_REQ_INTERVAL_8MF"},
	{ 1, 0x03, 0x00, "TS_PKT_SIZE_188"},
	{ 1, 0x03, 0x01, "TS_PKT_SIZE_376"},
	{ 1, 0x03, 0x02, "TS_PKT_SIZE_564"},
	{ 1, 0x03, 0x03, "TS_PKT_SIZE_752"},
	{ 6, 0x80, 0x80, "ACK_LAST_READ"},
	{ 6, 0x40, 0x40, "WAIT_ENABLE"},
	{ 6, 0x08, 0x08, "EEPROM_ON_BOARD"},
	{ 6, 0x04, 0x04, "EEPROM_KEY_VALID/BUS_SELECT"},
	{ 6, 0x03, 0x00, "FREQ_100_KHZ"},
	{ 6, 0x03, 0x01, "FREQ_400_KHZ"},
	{ 6, 0x03, 0x02, "FREQ_25_KHZ"},
	{ 6, 0x03, 0x03, "FREQ_1.5_MHZ"},
	{12, 0x20, 0x20, "SNAPSHOT_RESET"},
	{14, 0xff, 0x80, "LINE"},
	{14, 0xff, 0xc0, "TUNER"},
	{15, 0x80, 0x80, "AUDIO_UNMUTE"},
	{15, 0x40, 0x40, "I2S_MSB_TIMING"},
	{15, 0x20, 0x20, "IR_RC5_MODE"},
	{15, 0x10, 0x10, "IR_NEC_CHK_PAR"},
	{15, 0x0f, 0x00, "FREQ_30_MHZ"},
	{15, 0x0f, 0x01, "FREQ_15_MHZ"},
	{15, 0x0f, 0x02, "FREQ_10_MHZ"},
	{15, 0x0f, 0x03, "FREQ_7.5_MHZ"},
	{15, 0x0f, 0x04, "FREQ_6_MHZ"},
	{15, 0x0f, 0x05, "FREQ_5_MHZ"},
	{15, 0x0f, 0x06, "FREQ_4.3_MHZ"},
	{15, 0x0f, 0x07, "FREQ_12_MHZ"},
	{15, 0x0f, 0x08, "FREQ_20_MHZ"},
	{15, 0x0f, 0x09, "FREQ_20_MHZ_2"},
	{15, 0x0f, 0x0a, "FREQ_48_MHZ"},
	{15, 0x0f, 0x0b, "FREQ_24_MHZ"},
	{16, 0xff, 0x08, "YUV422_YUYV"},
	{16, 0xff, 0x09, "YUV422_YVYU"},
	{16, 0xff, 0x0a, "YUV422_UYVY"},
	{16, 0xff, 0x0b, "YUV422_VYUY"},
	{16, 0xff, 0x0c, "RGB8_BGGR"},
	{16, 0xff, 0x0d, "RGB8_GRBG"},
	{16, 0xff, 0x0e, "RGB8_GBRG"},
	{16, 0xff, 0x0f, "RGB8_RGGB"},
	{16, 0xff, 0x10, "YUV422_CbYCrY"},
	{17, 0x80, 0x80, "VBI_SLICED"},
	{17, 0x40, 0x40, "VBI_RAW"},
	{17, 0x20, 0x20, "VOUT_MODE_IN"},
	{17, 0x10, 0x10, "CCIR656_ENABLE"},
	{17, 0x08, 0x08, "VBI_16BIT"},
	{17, 0x04, 0x04, "FID_ON_HREF"},
	{17, 0x02, 0x02, "DUAL_EDGE_STROBE"},
	{17, 0x01, 0x01, "INTERLACED"},
	{39, 0xff, 0x00, "RGB8_RGRG"},
	{39, 0xff, 0x01, "RGB8_GRGR"},
	{39, 0xff, 0x02, "RGB8_GBGB"},
	{39, 0xff, 0x03, "RGB8_BGBG"},
	{39, 0xff, 0x04, "RGB16_656"},
	{39, 0xff, 0x08, "RGB8_BAYER"},
	{39, 0xff, 0x10, "YUV211"},
	{39, 0xff, 0x14, "YUV422_Y0UY1V"},
	{39, 0xff, 0x15, "YUV422_Y1UY0V"},
	{39, 0xff, 0x18, "YUV411"},
	{80, 0xff, 0x00, "NEC"},
	{80, 0xff, 0x01, "NEC_NO_PARITY"},
	{80, 0xff, 0x04, "RC5"},
	{80, 0xff, 0x08, "RC6_MODE_0"},
	{80, 0xff, 0x0b, "RC6_MODE_6A"},
	{95, 0x01, 0x01, "TS1_CAPTURE"},
	{95, 0x02, 0x02, "TS1_FILTER"},
	{95, 0x04, 0x04, "TS1_NULL_DISCARD"},
	{95, 0x10, 0x10, "TS2_CAPTURE"},
	{95, 0x20, 0x20, "TS2_FILTER"},
	{95, 0x40, 0x40, "TS2_NULL_DISCARD"}
};
const int N_REG_VALS = sizeof(em28xx_reg_values) / sizeof(struct Reg_Value);

struct Packet {
	u16 hdr_sz;
	u64 id;
	int status;
	int func;
	int dir;
	int bus;
	int port;
	int endpoint;
	int type;
	u32 pkt_sz;

	int len;
	u8 *data;

// URB_ISOC specific
	u32 start;
	u32 packs;
	u32 per_pack;
	u32 errors;

// URB_CTRL specific
	int stage;
	int req_type;
	int req;
	int value;
	int index;
};

int get_next_packet(struct Packet *pkt, u8 *buf, int idx) {
	pkt->hdr_sz   = *(u16*)&buf[idx];
	pkt->id       = *(u64*)&buf[idx+2];
	pkt->status   = *(int*)&buf[idx+10];
	pkt->func     = *(short*)&buf[idx+14];
	pkt->dir      = buf[idx+16];
	pkt->bus      = *(short*)&buf[idx+17];
	pkt->port     = *(short*)&buf[idx+19];
	pkt->endpoint = buf[idx+21];
	pkt->type     = buf[idx+22];
	pkt->pkt_sz   = *(u32*)&buf[idx+23];

	if (pkt->type == URB_ISOC) {
		pkt->start  = *(u32*)&buf[idx+27];
		pkt->packs  = *(u32*)&buf[idx+31];
		pkt->errors = *(u32*)&buf[idx+35];

		if (pkt->packs > 1)
			pkt->per_pack = *(u32*)&buf[idx+51]; // offset of the second packet
		else
			pkt->per_pack = 0;

		if ((pkt->endpoint & 0x80) == 0) {
			pkt->len = pkt->pkt_sz;
			pkt->data = &buf[idx + pkt->hdr_sz];
		}
	}
	else if (pkt->type == URB_CTRL && pkt->pkt_sz >= 8) {
		pkt->stage    = buf[idx+27];
		pkt->req_type = buf[idx+28];
		pkt->req      = buf[idx+29];
		pkt->value    = *(u16*)&buf[idx+30];
		pkt->index    = *(u16*)&buf[idx+32];
		pkt->len      = *(u16*)&buf[idx+34];
		pkt->data     = &buf[idx+36];
	}
	else if (pkt->type == URB_INTR || pkt->type == URB_BULK) {
		pkt->len  = pkt->pkt_sz;
		pkt->data = &buf[idx+27];
	}

	int size = pkt->hdr_sz + pkt->pkt_sz;
	size = size < 0xffff ? size : 0xffff;

	const int metadata_len = 0x10;
	return size + metadata_len;
}

void view_any(struct Packet *pkt, u8 *data, int count, int all) {
	const char *dir = (pkt->dir & 1) ? "<-" : "->";
	const char *end = (pkt->endpoint & 0x80) ? "Input" : "Output";
	int point = pkt->endpoint & 0x7f;

	char *xfer;
	char xfer_buf[8];
	switch (pkt->type) {
		case 0:
			xfer = "Isoc";
			break;
		case 1:
			xfer = "Intr";
			break;
		case 2:
			xfer = "Ctrl";
			break;
		case 3:
			xfer = "Bulk";
			break;
		default:
			sprintf(xfer_buf, "%d", pkt->type);
			xfer = &xfer_buf[0];
			break;
	}

	printf(
		"%04d %s %s - EP %d, Xfer %s, Func 0x%X, Size %6d",
		count, dir, end, point, xfer, pkt->func, pkt->pkt_sz
	);

	if (pkt->type == URB_ISOC) {
		printf(", Frame %8d, Pkts %3d, Errs %3d", pkt->start, pkt->packs, pkt->errors);
	}

	putchar('\n');
}

void view_ctrl_packet(struct Packet *pkt, u8 *data, int count, int all) {
	if (pkt->type != URB_CTRL) {
		if (all)
			view_any(pkt, data, count, all);

		return;
	}

	u8 *content = NULL;
	int pkt_size = pkt->pkt_sz;

	if (pkt->endpoint & 0x80) {
		if ((pkt->dir & 1) == 0) {
			printf("%04d read %d,%#x : ", count, pkt->req, pkt->index);
			return;
		}

		content = data + pkt->hdr_sz;
	}
	else {
		if (pkt->pkt_sz < 8)
			return;

		printf("%04d write %d,%#x : ", count, pkt->req, pkt->index);
		content = pkt->data;
		pkt_size = pkt->pkt_sz - 8;
	}

	for (int i = 0; i < pkt_size; i++)
		printf("%02x ", *content++);

	putchar('\n');
}

int prev_reg = 0;

void view_reg_packet(struct Packet *pkt, u8 *data, int count, int all) {
	if (pkt->type != URB_CTRL) {
		if (all)
			view_any(pkt, data, count, all);

		return;
	}

	int reg = pkt->index;
	if ((pkt->endpoint & 0x80) && (pkt->dir & 1))
		reg = prev_reg;

	char *reg_str = NULL;
	for (int i = 0; i < N_REGS; i++) {
		if (em28xx_regs[i].index == pkt->index) {
			reg_str = (char*)em28xx_regs[i].str;
			break;
		}
	}

	u8 *content = NULL;
	int pkt_size = pkt->pkt_sz;
	int use_label = reg_str != NULL && pkt->req == 0;

	if (pkt->endpoint & 0x80) {
		if ((pkt->dir & 1) == 0) {
			if (use_label)
				printf("%04d read %s : ", count, reg_str);
			else
				printf("%04d read %#x : ", count, pkt->index);

			prev_reg = pkt->index;
			return;
		}

		content = data + pkt->hdr_sz;
	}
	else {
		if (pkt->pkt_sz < 8)
			return;

		if (use_label)
			printf("%04d write %s : ", count, reg_str);
		else
			printf("%04d write %#x : ", count, pkt->index);

		content = pkt->data;
		pkt_size = pkt->pkt_sz - 8;
	}

	if (use_label && pkt_size == 1) {
		int n = *content;
		int i = 0;
		while (i < N_REG_VALS) {
			if (em28xx_reg_values[i].reg == reg)
				break;
			i++;
		}
		if (i >= N_REG_VALS) {
			printf("0x%02x\n", n);
			return;
		}

		struct Reg_Value *value = &em28xx_reg_values[i];

		int first = 1;
		while (i < N_REG_VALS) {
			if (value->reg != reg)
				break;

			if ((n & value->mask) == value->value) {
				printf("%s%s", first ? "" : " | ", value->str);
				n &= ~value->mask;
				first = 0;
			}

			i++;
			value++;
		}
		if (n || first)
			printf("%s0x%02x", first ? "" : " | ", n);
	}
	else {
		for (int i = 0; i < pkt_size; i++)
			printf("0x%02x ", *content++);
	}

	putchar('\n');
}

void view_command(struct Packet *pkt, u8 *data, int count, int all) {
	// skip all responses, process requests only
	if (pkt->dir & 1)
		return;

	if (pkt->type == URB_CTRL && pkt->req == SET_INTERFACE) {
		// in this case, index means interface and value means alt setting
		printf("select %d %d\n", pkt->index, pkt->value);
		return;
	}

	const char *types[] = {
		"isoc", "int", "ctrl", "bulk"
	};
	if (pkt->type < 0 || pkt->type > 3)
		printf("???");
	else
		printf("%s ", types[pkt->type]);

	int i;
	switch (pkt->type) {
		case URB_ISOC:
			printf("%#x %#x %#x", pkt->endpoint, pkt->packs, pkt->per_pack);
			if ((pkt->endpoint & 0x80) == 0) {
				for (i = 0; i < pkt->len; i++)
					printf(" %02x", pkt->data[i]);
			}
			break;
		case URB_INTR:
		case URB_BULK:
			if (pkt->endpoint & 0x80)
				printf("%#x %#x", pkt->endpoint, pkt->len);
			else {
				printf("%#x ", pkt->endpoint);
				for (i = 0; i < pkt->len; i++)
					printf("%02x ", pkt->data[i]);
			}
			break;
		case URB_CTRL:
			printf("%#x %d %#x %#x ", pkt->req_type, pkt->req, pkt->value, pkt->index);
			if (pkt->req_type & 0x80)
				printf("%#x", pkt->len);
			else {
				for (i = 0; i < pkt->len; i++)
					printf("%02x ", pkt->data[i]);
			}
			break;
	}

	putchar('\n');
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf(
			"USBPcap Analyser for EM28XX\n"
			"Usage: %s <option> <PCAP file>\n"
			"Options:\n"
			" -a\n"
			"   View all packet entries\n"
			" -c\n"
			"   View URB_CONTROL entries as general reads & writes\n"
			" -C\n"
			"   Same as -c but includes non-URB_CONTROL entries\n"
			" -r\n"
			"   View URB_CONTROL entries as reads & writes to EM28XX registers\n"
			" -R\n"
			"   Same as -r but includes non-URB_CONTROL entries\n"
			" -s\n"
			"   View packet entries formatted as commands\n",
			argv[0]
		);
		return 1;
	}

	void (*view)(struct Packet*, u8*, int, int);

	int all = 0;
	switch (argv[1][1]) {
		case 'A':
		case 'a':
			view = view_any;
			break;
		case 'C':
			all = 1;
		case 'c':
			view = view_ctrl_packet;
			break;
		case 'R':
			all = 1;
		case 'r':
			view = view_reg_packet;
			break;
		case 's':
			view = view_command;
			break;
		default:
			fprintf(stderr, "Unrecognised option \"%s\"\n", argv[1]);
			return 2;
	}

	FILE *f = fopen(argv[2], "rb");
	if (!f) {
		fprintf(stderr, "Could not open \"%s\"\n", argv[2]);
		return 3;
	}

	fseek(f, 0, SEEK_END);
	int sz = ftell(f);
	rewind(f);

	if (sz < 68) {
		fprintf(stderr, "\"%s\" is too small\n", argv[2]);
		fclose(f);
		return 4;
	}

	u8 *buf = malloc(sz);
	fread(buf, 1, sz, f);
	fclose(f);

	struct Packet pkt;
	u32 idx = 0x28;
	int count = 0;
	while (idx < sz) {
		int len = get_next_packet(&pkt, buf, idx);
		idx += len;
		count++;

		view(&pkt, buf + idx - len, count, all);
	}

	free(buf);
	return 0;
}
