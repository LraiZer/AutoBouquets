#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <string.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#define DVB_BUFFER_SIZE 2*4096

using namespace std;

template <class T>
string to_string(T t, ios_base & (*f)(ios_base&))
{
  ostringstream oss;
  oss << f << t;
  return oss.str();
}

u_int32_t crc_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

u_int32_t crc32 (const char *d, int len, u_int32_t crc) {
	register int i;
	const unsigned char *u=(unsigned char*)d;

	for (i=0; i<len; i++)
		crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *u++)];

	return crc;
}

void stringToUpper(string &s)
{
   for(unsigned int l = 0; l < s.length(); l++)
    s[l] = toupper(s[l]);
}

string SYMLINK_NAME(const char * s) {
	string r;
	while ((*s) != 0)
	{
		switch (*s)
		{
			case '&':
				r += "and";
				break;
			case '+':
				r += "plus";
				break;
			case '*':
				r += "star";
				break;
			default:
				if (isalnum(*s)) r += tolower(*s);
		}
		s++;
	}
	return r;
}

string Latin1_to_UTF8(const char * s)
{
	string r;
	while((*s) != 0)
	{
		unsigned char c = *s;
		if (c < 0x80)
			r += c;
		else
		{
			unsigned char d = 0xc0 | (c >> 6);
			r += d;
			d = 0x80 | (c & 0x3f);
			r += d;
		}
		s++;
	}
	return r;
}

struct header_t {
	unsigned short table_id;
	unsigned short variable_id;
	short version_number;
	short current_next_indicator;
	short section_number;
	short last_section_number;
} header;

struct sections_t {
	short version;
	short last_section;
	short received_section[0xff];
	short populated;
} Sections;

struct transport_t {
	unsigned short original_network_id;
	unsigned short transport_stream_id;
	short modulation_system;
	unsigned int frequency;
	unsigned int symbol_rate;
	short polarization;
	short modulation_type;
	short fec_inner;
	short roll_off;
	short orbital_position;
	short west_east_flag;
} Transport;

struct channel_t {
	string skyid;
	string provider;
	string name;
	string type;
	string onid;
	string tsid;
	string sid;
	string ca;
	string nspace;
} channel;

sections_t BAT1_SECTIONS, BAT2_SECTIONS, BAT3_SECTIONS, NIT_SECTIONS;
map<string, channel_t> BAT1, BAT2, BAT3, SDT, TV, RADIO, DATA, TEST;
map<string, transport_t> NIT;

// default Granada HD >< Northern Ireland HD
int BAT_local = 0x1005, BAT_local_region = 0x7;
int BAT_merge = 0x1008, BAT_merge_region = 0x21;
int skyid, fta = 2;
bool dvbloop = true;
unsigned short sdtmax = 0;

string prog_path() {
	int index;
	char buff[256];
	memset(buff, '\0', 256);
	ssize_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
	if (len != -1) {
		string bpath = string(buff);
		index = bpath.find_last_of("/\\");
		return bpath.substr(0, index);
	}
	else
		return "/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets";
}

int si_parse_nit(unsigned char *data, int length) {

	if (length < 8)
		return -1;

	int network_descriptors_length = ((data[8] & 0x0f) << 8) | data[9];
	int transport_stream_loop_length = ((data[network_descriptors_length + 10] & 0x0f) << 8) | data[network_descriptors_length + 11];
	int offset1 = network_descriptors_length + 12;

	while (transport_stream_loop_length > 0)
	{
		unsigned short tsid;
 		tsid = (data[offset1] << 8) | data[offset1 + 1];
		Transport.original_network_id = (data[offset1 + 2] << 8) | data[offset1 + 3];

		int transport_descriptor_length = ((data[offset1 + 4] & 0x0f) << 8) | data[offset1 + 5];
		int offset2 = offset1 + 6;

		offset1 += (transport_descriptor_length + 6);
		transport_stream_loop_length -= (transport_descriptor_length + 6);

		while (transport_descriptor_length > 0)
		{
			unsigned char descriptor_tag = data[offset2];
			unsigned char descriptor_length = data[offset2 + 1];

			if (descriptor_tag == 0x43)
			{
				Transport.frequency = (data[offset2 + 2] >> 4) * 10000000;
				Transport.frequency += (data[offset2 + 2] & 0x0f) * 1000000;
				Transport.frequency += (data[offset2 + 3] >> 4) * 100000;
				Transport.frequency += (data[offset2 + 3] & 0x0f) * 10000;
				Transport.frequency += (data[offset2 + 4] >> 4) * 1000;
				Transport.frequency += (data[offset2 + 4] & 0x0f) * 100;
				Transport.frequency += (data[offset2 + 5] >> 4) * 10;
				Transport.frequency += data[offset2 + 5] & 0x0f;
				
				Transport.orbital_position = (data[offset2 + 6] << 8) | data[offset2 + 7];
				Transport.west_east_flag = (data[offset2 + 8] >> 7) & 0x01;
				Transport.polarization = (data[offset2 + 8] >> 5) & 0x03;
				Transport.roll_off = (data[offset2 + 8] >> 3) & 0x03;
				Transport.modulation_system = (data[offset2 + 8] >> 2) & 0x01;
				Transport.modulation_type = data[offset2 + 8] & 0x03;

				Transport.symbol_rate = (data[offset2 + 9] >> 4) * 1000000;
				Transport.symbol_rate += (data[offset2 + 9] & 0xf) * 100000;
				Transport.symbol_rate += (data[offset2 + 10] >> 4) * 10000;
				Transport.symbol_rate += (data[offset2 + 10] & 0xf) * 1000;
				Transport.symbol_rate += (data[offset2 + 11] >> 4) * 100;
				Transport.symbol_rate += (data[offset2 + 11] & 0xf) * 10;
				Transport.symbol_rate += data[offset2 + 11] >> 4;
				
				Transport.fec_inner = data[offset2 + 12] & 0xf;

				NIT[to_string<unsigned short>(tsid, hex)] = Transport;
			}
			
			offset2 += (descriptor_length + 2);
			transport_descriptor_length -= (descriptor_length + 2);
		}
	}
	return 1;
}

int sections_check(sections_t *sections) {
	for ( int i = 0; i <= header.last_section_number; i++ ) {
		if ( sections->received_section[i] == 0 )
			return 0;
	}
	return 1;
}

void network_check(int variable_id, sections_t *sections, unsigned char *data, int length) {
	if (!sections->received_section[header.section_number])
	{
		if (si_parse_nit(data, length))
		{
			sections->received_section[header.section_number] = 1;
			if (sections_check(sections))
				sections->populated = 1;
		}
	}
}

int si_open(int dvb_frontend, int dvb_adapter, int dvb_demux, int pid) {
	short fd = 0;
	char demuxer[256];
	memset(demuxer, '\0', 256);
	sprintf(demuxer, "/dev/dvb/adapter%i/demux%i", dvb_adapter, dvb_demux);

	char filter, mask;
	struct dmx_sct_filter_params sfilter;
	dmx_source_t ssource = DMX_SOURCE_FRONT0;

	filter = 0x40;
	mask = 0xf0;

	memset(&sfilter, 0, sizeof(sfilter));
	sfilter.pid = pid & 0xffff;
	sfilter.filter.filter[0] = filter & 0xff;
	sfilter.filter.mask[0] = mask & 0xff;
	sfilter.timeout = 0;
	sfilter.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	switch(dvb_frontend)
	{
	case 1:
		ssource = DMX_SOURCE_FRONT1;
		break;
	case 2:
		ssource = DMX_SOURCE_FRONT2;
		break;
	case 3:
		ssource = DMX_SOURCE_FRONT3;
		break;
	default:
		ssource = DMX_SOURCE_FRONT0;
		break;
	}

	if ((fd = open(demuxer, O_RDWR | O_NONBLOCK)) < 0)
		printf("Cannot open demuxer '%s'\n", demuxer );

	// STB frontend default, check if manual override
	if (dvb_frontend != -1)
	{
		if (ioctl(fd, DMX_SET_SOURCE, &ssource) == -1) {
			printf("ioctl DMX_SET_SOURCE failed\n");
			close(fd);
		}
	}

	if (ioctl(fd, DMX_SET_FILTER, &sfilter) == -1) {
		printf("ioctl DMX_SET_FILTER failed\n");
		close(fd);
	}

	return fd;
}

void si_close(int fd) {
	if (fd > 0) {
		ioctl (fd, DMX_STOP, 0);
		close(fd);
	}
}

void si_parse_header(unsigned char *data) {
	header.table_id = data[0];
	header.variable_id = (data[3] << 8) | data[4];
	header.version_number = (data[5] >> 1) & 0x1f;
	header.current_next_indicator = data[5] & 0x01;
	header.section_number = data[6];
	header.last_section_number = data[7];
}

void si_parse_sdt(unsigned char *data, int length) {
	unsigned short transport_stream_id = (data[3] << 8) | data[4];
	unsigned short original_network_id = (data[8] << 8) | data[9];

	int offset = 11;
	length -= 11;

	while (length >= 5)
	{
		unsigned short service_id = (data[offset] << 8) | data[offset + 1];
		short free_ca = (data[offset + 3] >> 4) & 0x01;
		int descriptors_loop_length = ((data[offset + 3] & 0x0f) << 8) | data[offset + 4];
		char service_name[256];
		char provider_name[256];
		unsigned short service_type = 0;
		memset(service_name, '\0', 256);
		memset(provider_name, '\0', 256);

		length -= 5;
		offset += 5;

		int offset2 = offset;

		length -= descriptors_loop_length;
		offset += descriptors_loop_length;

		while (descriptors_loop_length >= 2)
		{
			int tag = data[offset2];
			int size = data[offset2 + 1];

			if (tag == 0x48) // service_descriptor
			{
				service_type = data[offset2 + 2];
				int service_provider_name_length = data[offset2 + 3];
				if (service_provider_name_length == 255)
					service_provider_name_length--;

				int service_name_length = data[offset2 + 4 + service_provider_name_length];
				if (service_name_length == 255)
					service_name_length--;

				memset(service_name, '\0', 256);
				memcpy(provider_name, data + offset2 + 4, service_provider_name_length);
				memcpy(service_name, data + offset2 + 5 + service_provider_name_length, service_name_length);
			}
			if (tag == 0xc0) // nvod + adult service descriptor
			{
				memset(service_name, '\0', 256);
				memcpy(service_name, data + offset2 + 2, size);
			}

			descriptors_loop_length -= (size + 2);
			offset2 += (size + 2);
		}

		char *provider_name_ptr = provider_name;
		if (strlen(provider_name) == 0)
			strcpy(provider_name, "BSkyB");
		else if (provider_name[0] == 0x05)
			provider_name_ptr++;

		char *service_name_ptr = service_name;
		if (strlen(service_name) == 0)
			strcpy(service_name, to_string<unsigned short>(service_id, dec).c_str());
		else if (service_name[0] == 0x05)
			service_name_ptr++;

		string sid = to_string<unsigned short>(service_id, hex);
		if (strlen(SDT[sid].provider.c_str()) == 0)
			SDT[sid].provider = provider_name_ptr;
		if (strlen(SDT[sid].name.c_str()) == 0) {
			SDT[sid].name = service_name_ptr;
		}

		SDT[sid].ca = free_ca ? "NDS" : "FTA";
		SDT[sid].type = to_string<unsigned short>(service_type, hex);
		SDT[sid].sid = to_string<unsigned short>(service_id, hex);
		SDT[sid].onid = to_string<unsigned short>(original_network_id, hex);
		SDT[sid].tsid = to_string<unsigned short>(transport_stream_id, hex);
	}
}

int si_parse_bat(unsigned char *data, int length) {

	if (length < 8)
		return -1;
	
	int bouquet_descriptors_length = ((data[8] & 0x0f) << 8) | data[9];
	int transport_stream_loop_length = ((data[bouquet_descriptors_length + 10] & 0x0f) << 8) | data[bouquet_descriptors_length + 11];
	int offset1 = 10;

	while (bouquet_descriptors_length > 0)
	{
		unsigned char descriptor_tag = data[offset1];
		unsigned char descriptor_length = data[offset1 + 1];

		if (descriptor_tag == 0x47)
		{
			char description[descriptor_length + 1];
			memset(description, '\0', descriptor_length + 1);
			memcpy(description, data + offset1 + 2, descriptor_length);
		}
		offset1 += (descriptor_length + 2);
		bouquet_descriptors_length -= (descriptor_length + 2);
	}

	offset1 += 2;

	while (transport_stream_loop_length > 0)
	{
		unsigned short transport_stream_id = (data[offset1] << 8) | data[offset1 + 1];
		unsigned short original_network_id = (data[offset1 + 2] << 8) | data[offset1 + 3];
		int transport_descriptor_length = ((data[offset1 + 4] & 0x0f) << 8) | data[offset1 + 5];
		int offset2 = offset1 + 6;

		offset1 += (transport_descriptor_length + 6);
		transport_stream_loop_length -= (transport_descriptor_length + 6);

		while (transport_descriptor_length > 0)
		{
			unsigned char descriptor_tag = data[offset2];
			unsigned char descriptor_length = data[offset2 + 1];
			int offset3 = offset2 + 2;

			offset2 += (descriptor_length + 2);
			transport_descriptor_length -= (descriptor_length + 2);

			if (descriptor_tag == 0xb1)
			{
				unsigned char region_id;
				region_id = data[offset3 + 1];
				
				offset3 += 2;
				descriptor_length -= 2;
				while (descriptor_length > 0)
				{
					unsigned short channel_id;
					unsigned short sky_id;
					unsigned short service_id;
					unsigned char service_type;
					string epg_id;

					channel_id = (data[offset3 + 3] << 8) | data[offset3 + 4];
					sky_id = ( data[offset3 + 5] << 8 ) | data[offset3 + 6];
					service_id = (data[offset3] << 8) | data[offset3 + 1];
					service_type = data[offset3 + 2];
					epg_id = to_string<unsigned short>(channel_id, dec);

					if ( header.variable_id == BAT_local && ( region_id == BAT_local_region || region_id == 0xff ))
					{
						BAT1[epg_id].skyid = to_string<unsigned short>(sky_id, dec);
						BAT1[epg_id].type = to_string<unsigned short>(service_type, hex);
						BAT1[epg_id].onid = to_string<unsigned short>(original_network_id, hex);
						BAT1[epg_id].tsid = to_string<unsigned short>(transport_stream_id, hex);
						BAT1[epg_id].sid = to_string<unsigned short>(service_id, hex);
					}
					else if ( header.variable_id == BAT_merge && ( region_id == BAT_merge_region || region_id == 0xff ))
					{
						BAT2[epg_id].skyid = to_string<unsigned short>(sky_id, dec);
						BAT2[epg_id].type = to_string<unsigned short>(service_type, hex);
						BAT2[epg_id].onid = to_string<unsigned short>(original_network_id, hex);
						BAT2[epg_id].tsid = to_string<unsigned short>(transport_stream_id, hex);
						BAT2[epg_id].sid = to_string<unsigned short>(service_id, hex);
					}
					else if ( header.variable_id == 0x100d && BAT3.find(epg_id) == BAT3.end() )
					{
						BAT3[epg_id].skyid = to_string<unsigned short>(sky_id, dec);
						BAT3[epg_id].type = to_string<unsigned short>(service_type, hex);
						BAT3[epg_id].onid = to_string<unsigned short>(original_network_id, hex);
						BAT3[epg_id].tsid = to_string<unsigned short>(transport_stream_id, hex);
						BAT3[epg_id].sid = to_string<unsigned short>(service_id, hex);
					}
					offset3 += 9;
					descriptor_length -= 9;
				}
			}
		}
	}
	return 1;
}

void bouquet_check(int variable_id, sections_t *sections, unsigned char *data, int length) {
	if (!sections->received_section[header.section_number])
	{
		if (si_parse_bat(data, length))
		{
			sections->received_section[header.section_number] = 1;
			if (sections_check(sections))
				sections->populated = 1;
		}
	}
}

int si_read_bouquets(int fd) {

	unsigned char buffer[DVB_BUFFER_SIZE];
	bool SDT_SECTIONS_populated = false;

	while ( !BAT1_SECTIONS.populated || !BAT2_SECTIONS.populated || 
	        !BAT3_SECTIONS.populated || !SDT_SECTIONS_populated ) {

		int size = read(fd, buffer, sizeof(buffer));

		if (size < 3) {
			usleep(100000);
			return -1;
		}
		
		int section_length = ((buffer[1] & 0x0f) << 8) | buffer[2];

		if (size != section_length + 3)
			return -1;

		int calculated_crc = crc32((char *) buffer, section_length + 3, 0xffffffff);

		if (calculated_crc)
			calculated_crc = 0;

		if (calculated_crc != 0)
			return -1;

		si_parse_header(buffer);

		if (header.table_id == 0x4a)
		{
			if ( !BAT1_SECTIONS.populated || !BAT2_SECTIONS.populated || !BAT3_SECTIONS.populated )
			{
				if ( header.variable_id == BAT_local )		// regional bat
					bouquet_check(BAT_local, &BAT1_SECTIONS, buffer, section_length);
				else if ( header.variable_id == BAT_merge )	// merge bat (english >< irish)
					bouquet_check(BAT_merge, &BAT2_SECTIONS, buffer, section_length);
				else if ( header.variable_id == 0x100d )	// testing bat
					bouquet_check(0x100d, &BAT3_SECTIONS, buffer, section_length);
			}
		}
		else if (header.table_id == 0x42 || header.table_id == 0x46)
		{
			si_parse_sdt(buffer, section_length);

			sdtmax++;
			if (sdtmax < 0x1f4)
				SDT_SECTIONS_populated = false;
			else
				SDT_SECTIONS_populated = true;
		}
	}

	if ( BAT1_SECTIONS.populated && BAT2_SECTIONS.populated && BAT3_SECTIONS.populated && SDT_SECTIONS_populated )
		return 1;
	else
		return 0;
}

int si_read_network(int fd) {

	unsigned char buffer[DVB_BUFFER_SIZE];

	int size = read(fd, buffer, sizeof(buffer));
	
	if (size < 3) {
		usleep(100000);
		return -1;
	}

	int section_length = ((buffer[1] & 0x0f) << 8) | buffer[2];

	if (size != section_length + 3)
		return -1;

	int calculated_crc = crc32((char *) buffer, section_length + 3, 0xffffffff);

	if (calculated_crc)
		calculated_crc = 0;

	if (calculated_crc != 0)
		return -1;

	si_parse_header(buffer);

	if ( header.variable_id == 0x20 )
		network_check(0x20, &NIT_SECTIONS, buffer, section_length);

	if ( NIT_SECTIONS.populated )
		return 1;

	return 0;
}

void write_bouquet_service(channel_t channel,ofstream& bq_stream,bool numbering,string sort_skyid,string bq_O,string bq_F,string bq_d,string bq_s,string bq_s2,string bq_P) {
	int is_type = atoi(channel.type.c_str());
	if ((fta == 2) && (channel.ca == "NDS") && ( is_type == 19 || is_type == 87 ))
		bq_stream << bq_P << endl << bq_d << " " << endl;
	else
	{
		bq_stream << bq_s << hex << channel.type << ":" << channel.sid << ":" << channel.tsid << bq_O << channel.nspace << bq_F << endl;
		if (numbering)
			bq_stream << bq_d << dec << sort_skyid << bq_s2 << channel.name << endl;
		else
			bq_stream << bq_d << dec << channel.name << endl;
	}
}

void write_bouquet_name(string bq_name,ofstream& bq_00,ofstream& bq_14,ofstream& bq_15,int custom_sort,string bq_S,string bq_d,string bq_n1,string bq_n2) {
	bq_14 << bq_S << endl << bq_d << bq_n1 << bq_name << bq_n2 << endl;
	bq_15 << bq_S << endl << bq_d << bq_n1 << bq_name << bq_n2 << endl;
	if (custom_sort != 1) bq_00 << bq_S << endl << bq_d << bq_n1 << bq_name << bq_n2 << endl;
}

void write_bouquet_names(int count,string bq_n01,string bq_n02,string bq_n03,string bq_n04,string bq_n05,string bq_n06,string bq_n07,
			string bq_n08,string bq_n09,string bq_n0a,string bq_n0b,string bq_n0c,string bq_n0d,string bq_n0e,string bq_n0f,string bq_n10,
			ofstream& bq_00,ofstream& bq_14,ofstream& bq_15,int custom_sort,string bq_S,string bq_d,string bq_n1,string bq_n2,bool parentalcontrol) {
	     if (count == 101) write_bouquet_name(bq_n01,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
//	else if (count == 240) write_bouquet_name(bq_n02,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);  //Reserved: Lifestyle and Culture
	else if (count == 301) write_bouquet_name(bq_n03,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 350) write_bouquet_name(bq_n04,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 401) write_bouquet_name(bq_n05,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 501) write_bouquet_name(bq_n06,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 520) write_bouquet_name(bq_n07,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 580) write_bouquet_name(bq_n08,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 601) write_bouquet_name(bq_n09,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 650) write_bouquet_name(bq_n0a,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 700) write_bouquet_name(bq_n0b,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 780) write_bouquet_name(bq_n0c,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 871 && !parentalcontrol) // parental control - gaming and dating
		               write_bouquet_name(bq_n0d,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 881) write_bouquet_name(bq_n0e,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 900 && !parentalcontrol) // parental control - adult
		               write_bouquet_name(bq_n0f,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
	else if (count == 950) write_bouquet_name(bq_n10,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2);
}

bool bouquet_has_service(string file_name) {
	char name_file[256];
	memset(name_file, '\0', 256);
	sprintf(name_file, "/tmp/userbouquet.ukcvs%s.tv", file_name.c_str());
	int number_of_lines = 0;
	std::string line;
	std::ifstream myfile(name_file);

	while (std::getline(myfile, line))
		++number_of_lines;

	if (number_of_lines < 4)
	{
		unlink(name_file);
		return 0;
	}
	return 1;
}

int main (int argc, char *argv[]) {

	const char *ABV = "AutoBouquetsReader Build " __TIMESTAMP__;

	time_t dvb_loop_start;
	int loop_time = 120;

	bool numbering = true, update = true, placeholder = true;
	bool parentalcontrol = true, custom_swap = false, extra = false;
	int fd, dvb_frontend = -1, dvb_adapter = 0, dvb_demux = 0;
	int custom_sort = 0, piconstyle = 0, lamedb_version = 4;
	string style = "0", piconlink = "0", piconfolder = "0", picon_link, picon_folder;

	char curr_path[256];
	memset(curr_path, '\0', 256);
	sprintf(curr_path, "%s", prog_path().c_str());

	if (argv[1] != NULL) {
	 BAT_local = atoi(argv[1]);
	 if (argv[2] != NULL) {
	  BAT_local_region = atoi(argv[2]);
	  if (BAT_local_region == 0x21 || BAT_local_region == 0x32) {
	   BAT_merge = 0x1005;
	   BAT_merge_region = 0x1;
	  }
	  if (argv[3] != NULL) {
	   extra = atoi(argv[3]);
	   if (argv[4] != NULL) {
	    custom_sort = atoi(argv[4]);
	    if (argv[5] != NULL) {
	     numbering = atoi(argv[5]);
	     if (argv[6] != NULL) {
	      update = atoi(argv[6]);
	      if (argv[7] != NULL) {
	       placeholder = atoi(argv[7]);
	       if (argv[8] != NULL) {
	        parentalcontrol = atoi(argv[8]);
	        if (argv[9] != NULL) {
	         custom_swap = atoi(argv[9]);
	         if (argv[10] != NULL) {
	          fta = atoi(argv[10]);
	          if (argv[11] != NULL) {
	           style = argv[11];
	           if (argv[12] != NULL) {
	            piconlink = argv[12];
	            if (argv[13] != NULL) {
	             piconfolder = argv[13];
	             if (argv[14] != NULL) {
	              piconstyle = atoi(argv[14]);
	              if (argv[15] != NULL) {
	               lamedb_version = atoi(argv[15]);
	               if (argv[16] != NULL) {
	                dvb_frontend = atoi(argv[16]);
	                if (argv[17] != NULL) {
	                 dvb_adapter = atoi(argv[17]);
	                 if (argv[18] != NULL) {
	                  dvb_demux = atoi(argv[18]);
	}}}}}}}}}}}}}}}}}}

	// Make Hex Editor Hackable so they dont need to recompile for changes ;-)
	string SkySportsActive1 = "1471"; unsigned short ssa1 = atoi(SkySportsActive1.c_str());
	string SkySportsActive2 = "1480"; unsigned short ssa2 = atoi(SkySportsActive2.c_str());
	string BTextra1 = "5030"; unsigned short btx1 = atoi(BTextra1.c_str());
	string BTextra2 = "5032"; unsigned short btx2 = atoi(BTextra2.c_str());
	string BTextra3 = "5380"; unsigned short btx3 = atoi(BTextra3.c_str());
	string BTextra4 = "5387"; unsigned short btx4 = atoi(BTextra4.c_str());
	string SkyAnytime1 = "4094"; unsigned short any1 = atoi(SkyAnytime1.c_str());
	string SkyAnytime2 = "4099"; unsigned short any2 = atoi(SkyAnytime2.c_str());

	if (update)
	{
		fd = si_open(dvb_frontend, dvb_adapter, dvb_demux, 0x10);

		dvb_loop_start = time(NULL);

		while(si_read_network(fd) < 1)
		{
			if (time(NULL) > dvb_loop_start + loop_time)
			{
				printf("[AutoBouquetsReader] read network timeout! %i seconds\n", loop_time);
				si_close(fd);
				return -1;
			}
		}

		si_close(fd);

		ofstream dat_nit;
		dat_nit.open ("/tmp/nit_transponders.txt");

		for( map<string, transport_t>::iterator i = NIT.begin(); i != NIT.end(); ++i )
		{
			if (lamedb_version == 5)
			{
				dat_nit << "t:" << ((*i).first == "7e3" ? "011a2f26" : "011a0000") << ":";
				dat_nit << hex << right;
				dat_nit << setw(4) << setfill('0') << (*i).first << ":";
				dat_nit << setw(4) << setfill('0') << (*i).second.original_network_id << ",s:";
				dat_nit << dec << left;
				dat_nit << setw(8) << setfill('0') << (*i).second.frequency << ":";
				dat_nit << setw(8) << setfill('0') << (*i).second.symbol_rate << ":";
				dat_nit << hex;
				dat_nit << (*i).second.polarization << ":" << (*i).second.fec_inner << ":" << (*i).second.orbital_position << ":";

				if ((*i).second.modulation_system == 1)
					dat_nit << "2:0:1:" << (*i).second.modulation_type << ":" << (*i).second.roll_off << ":2";
				else
					dat_nit << "2:0";

				dat_nit << endl;
			}
			else	// lamedb version 4
			{
				dat_nit << ((*i).first == "7e3" ? "011a2f26" : "011a0000") << ":";
				dat_nit << hex << right;
				dat_nit << setw(4) << setfill('0') << (*i).first << ":";
				dat_nit << setw(4) << setfill('0') << (*i).second.original_network_id << endl << "\ts ";
				dat_nit << dec << left;
				dat_nit << setw(8) << setfill('0') << (*i).second.frequency << ":";
				dat_nit << setw(8) << setfill('0') << (*i).second.symbol_rate << ":";
				dat_nit << hex;
				dat_nit << (*i).second.polarization << ":" << (*i).second.fec_inner << ":" << (*i).second.orbital_position << ":";

				if ((*i).second.modulation_system == 1)
					dat_nit << "2:0:1:" << (*i).second.modulation_type << ":" << (*i).second.roll_off << ":2";
				else
					dat_nit << "2:0";

				dat_nit << endl << "/" << endl;
			}
		}

		dat_nit.close();
		NIT.clear();
	}

	fd = si_open(dvb_frontend, dvb_adapter, dvb_demux, 0x11);

	dvb_loop_start = time(NULL);

	while(si_read_bouquets(fd) < 1)
	{
		if (time(NULL) > dvb_loop_start + loop_time)
		{
			printf("[AutoBouquetsReader] read bouquets timeout! %i seconds\n", loop_time);
			si_close(fd);
			return -1;
		}
	}

	si_close(fd);

	pair<map<string, channel_t>::iterator,bool> ret;

	for( map<string, channel_t>::iterator ii = BAT2.begin(); ii != BAT2.end(); ++ii )
	{
		bool foundskyid = false;
		for( map<string, channel_t>::iterator i = BAT1.begin(); i != BAT1.end(); ++i )
		{
			if ( (*i).second.skyid == (*ii).second.skyid )
			{
				foundskyid = true;
				break;
			}
		}

		if ( !foundskyid )
		{
			ret = BAT1.insert ( pair<string, channel_t>((*ii).first,channel) );
			if (ret.second == true)
				BAT1[(*ii).first] = BAT2[(*ii).first];
		}
	}

	BAT2.clear();

	// remove duplicates from bat3 that are already in merged bat1
	for( map<string, channel_t>::iterator i = BAT1.begin(); i != BAT1.end(); ++i )
	{
		if (BAT3.find((*i).first) != BAT3.end())
			BAT3.erase ((*i).first);
	}

	// map remaining BAT3 services into TEST and DATA
	for( map<string, channel_t>::iterator ii = BAT3.begin(); ii != BAT3.end(); ++ii )
	{
		string sid = to_string<string>((*ii).second.sid, hex);
		if ((fta == 1 && SDT[sid].ca == "FTA") || fta != 1)
		{
			unsigned short skyid = atoi((*ii).second.skyid.c_str());

			// hack to re-assign channel numbers for duplicate regionals in transmitted TEST bouquet services
			switch (skyid)
			{
				case 131: // ITV +1
					(*ii).second.skyid = "69";
					break;
			}

			if (skyid != 65535)
			{
				bool foundskyid = false;
				for( map<string, channel_t>::iterator i = BAT1.begin(); i != BAT1.end(); ++i )
				{
					if ( (*i).second.skyid == (*ii).second.skyid )
					{
						foundskyid = true;
						break;
					}
				}

				if ((!foundskyid ) && (skyid > 100 && skyid < 1000))
				{
					ret = BAT1.insert ( pair<string, channel_t>((*ii).first,channel) );
					if (ret.second == true)
						BAT1[(*ii).first] = BAT3[(*ii).first];
				}
				else
				{
					// hack to re-assign channel numbers to 9k+ for duplicate regionals in transmitted TEST bouquet services
					string test_id = (*ii).second.skyid;
					if (skyid > 99 && skyid < 1000)
						test_id = ("9"+(*ii).second.skyid);

					TEST[test_id] = BAT3[(*ii).first];
					TEST[test_id].skyid = (*ii).first;
					TEST[test_id].ca       = SDT[sid].ca;
					TEST[test_id].provider = SDT[sid].provider;
					TEST[test_id].name     = Latin1_to_UTF8(SDT[sid].name.c_str());
					TEST[test_id].nspace   = ((*ii).second.tsid == "7e3" ? "11a2f26" : "11a0000");
				}
			}
			else // detect unassigned skyid as DATA
			{
				DATA[(*ii).first] = BAT3[(*ii).first];
				DATA[(*ii).first].ca       = SDT[sid].ca;
				DATA[(*ii).first].provider = SDT[sid].provider;
				DATA[(*ii).first].name     = Latin1_to_UTF8(SDT[sid].name.c_str());
				DATA[(*ii).first].nspace   = ((*ii).second.tsid == "7e3" ? "11a2f26" : "11a0000");
			}
		}
	}

	BAT3.clear();

	// add sdt data to merged bat1
	string sid;
	for( map<string, channel_t>::iterator i = BAT1.begin(); i != BAT1.end(); ++i )
	{
		sid = to_string<string>((*i).second.sid, hex);
		if ((fta == 1 && SDT[sid].ca == "FTA") || fta != 1)
		{
			BAT1[(*i).first].ca       = SDT[sid].ca;
			BAT1[(*i).first].provider = SDT[sid].provider;
			BAT1[(*i).first].name     = Latin1_to_UTF8(SDT[sid].name.c_str());
			BAT1[(*i).first].nspace   = ((*i).second.tsid == "7e3" ? "11a2f26" : "11a0000");
		}
		else
			BAT1.erase((*i).first);
	}

	SDT.clear();

	int count;

	string bq_O = ":2:";
	string bq_F = ":0:0:0:";
	string bq_S = "#SERVICE 1:64:1:0:0:0:0:0:0:0:";
	string bq_P = "#SERVICE 1:832:d:0:0:0:0:0:0:0:";

	if (!placeholder)
		bq_P = "#SERVICE 1:0:1:2331:7ee:2:11a0000:0:0:0:";

	string bq_D = "#DESCRIPTION 28.2E ";
	string bq_N = "#NAME ";
	string bq_s = "#SERVICE 1:0:";
	string bq_d = "#DESCRIPTION ";
	string bq_n1 = "-- ";
	string bq_n2 = " --";

	string bq_s1 = "  - -  ";
	string bq_s2 = " - ";
	switch (style[0])
	{
	  case '1':
		bq_s2 = " = ";
		break;
	  case '2':
		bq_s1 = "";
		break;
	  case '3':
		bq_s1 = "";
		bq_s2 = " = ";
		break;
	  case '4':
		bq_s1 = "  = =  ";
		bq_s2 = " = ";
		break;
	  case '5':
		bq_s1 = "";
		bq_s2 = " ";
		break;
	  default:
		break;
	}

	switch (piconlink[0])
	{
	  case '1':
		picon_link = "/usr/share/enigma2/picon/";
		break;
	  case '2':
		picon_link = "/picon/";
		break;
	  case '3':
		picon_link = "/media/usb/picon/";
		break;
	  case '4':
		picon_link = "/media/hdd/picon/";
		break;
	  case '5':
		picon_link = "/media/cf/picon/";
		break;
	  default:
		picon_link = "/tmp/";
		break;
	}

	switch (piconfolder[0])
	{
	  case '1':
		picon_folder = "/usr/share/enigma2/picon/";
		break;
	  case '2':
		picon_folder = "/picon/";
		break;
	  case '3':
		picon_folder = "/media/usb/picon/";
		break;
	  case '4':
		picon_folder = "/media/hdd/picon/";
		break;
	  case '5':
		picon_folder = "/media/cf/picon/";
		break;
	  default:
		picon_folder = picon_link;
		break;
	}

	string bouquets_ntv1 = "#SERVICE 1:7:1:0:0:0:0:0:0:0:FROM BOUQUET \"userbouquet.ukcvs";
	string bouquets_ntv2 = ".tv\" ORDER BY bouquet";

	string bq_n00 = "28.2E UK Bouquets";
	string bq_n01 = "Entertainment";
	string bq_n02 = "Lifestyle and Culture";
	string bq_n03 = "Movies";
	string bq_n04 = "Music";
	string bq_n05 = "Sports";
	string bq_n06 = "News";
	string bq_n07 = "Documentaries";
	string bq_n08 = "Religious";
	string bq_n09 = "Kids";
	string bq_n0a = "Shopping";
	string bq_n0b = "Sky Box Office";
	string bq_n0c = "International";
	string bq_n0d = "Gaming and Dating";
	string bq_n0e = "Specialist";
	string bq_n0f = "Adult";
	string bq_n10 = "Other";
	string bq_n11 = "Sky Sports Interactive";
	string bq_n12 = "BT Sports Interactive";
	string bq_n13 = "Sky Anytime";
	string bq_n14 = "Free To Air";
	string bq_n15 = "High Definition";
	string bq_nu1 = "User Bouquet 1";
	string bq_nu2 = "User Bouquet 2";
	string bq_nu3 = "User Bouquet 3";
	string bq_nu4 = "User Bouquet 4";
	string bq_nu5 = "User Bouquet 5";
	string bq_rgl = "Local Region";
	string bq_tst = "Extra Channels";
	string bq_dat = "Extra Services";
	string bq_nradio = "RADIO";

	unsigned int ch;
	char *hex_str = new char[256];

	string datfile;
	datfile = "/tmp/autobouquets.csv";
	ofstream database_csv;
	database_csv.open (datfile.c_str());
	database_csv << "\"POSITION\",\"EPG_ID\",\"TYPE\",\"SID\",\"TSID\",\"ENCRYPTION\",\"NAME\",\"REF\"," << ABV << endl;

	datfile = "/tmp/lamedb_services.txt";
	ofstream lamedb_services;
	lamedb_services.open (datfile.c_str());

	for( map<string, channel_t>::iterator i = BAT1.begin(); i != BAT1.end(); ++i )
	{
		if ((fta == 1 && (*i).second.ca == "FTA") || fta != 1)
		{
			if (update)
			{
				hex_str[(*i).second.type.size()] = 0;
				memcpy(hex_str, (*i).second.type.c_str(), (*i).second.type.size());
				sscanf( hex_str, "%x", &ch );

				if (lamedb_version == 5)
				{
					lamedb_services << "s:" << hex;
					lamedb_services << setw(4) << setfill('0') << (*i).second.sid << ":0" << (*i).second.nspace << ":";
					lamedb_services << setw(4) << setfill('0') << (*i).second.tsid << ":0002:" << dec << ch << ":0,\"";
					lamedb_services << dec << (*i).second.name << "\",p:" << (*i).second.provider << endl;
				}
				else	// lamedb version 4
				{
					lamedb_services << hex;
					lamedb_services << setw(4) << setfill('0') << (*i).second.sid << ":0" << (*i).second.nspace << ":";
					lamedb_services << setw(4) << setfill('0') << (*i).second.tsid << ":0002:" << dec << ch << ":0" << endl;
					lamedb_services << dec << (*i).second.name << endl << "p:" << (*i).second.provider << endl;
				}
			}

			unsigned short skyid = atoi((*i).second.skyid.c_str());

			if (skyid > 100 && skyid < 1000)
			{
				TV[(*i).second.skyid] = BAT1[(*i).first];
				TV[(*i).second.skyid].skyid = (*i).first;
			}
			else if (skyid > 3100 && skyid < 4000)
			{
				RADIO[(*i).second.skyid] = BAT1[(*i).first];
				RADIO[(*i).second.skyid].skyid = (*i).first;
			}
			else if (skyid == 0xffff)
				DATA[(*i).first] = BAT1[(*i).first];
			else
			{
				TEST[(*i).second.skyid] = BAT1[(*i).first];
				TEST[(*i).second.skyid].skyid = (*i).first;
			}
		}
	}

	lamedb_services.close();

	BAT1.clear();

	if (!update)
		remove(datfile.c_str());

	char name_file[256];
	memset(name_file, '\0', 256);

	if (custom_sort > 1)
		sprintf(name_file, "%s/custom_sort_%i.txt", curr_path, custom_sort-1);
	else
		sprintf(name_file, "%s/supplement.txt", curr_path);

	ifstream custom_name_file(name_file);

	if (custom_name_file.is_open())
	{
		string dat_line;
		while (!custom_name_file.eof() && getline(custom_name_file, dat_line))
		{
			if (dat_line.length() > 0 && dat_line[0] == '$')
			{
				int index = dat_line.find('=');
				string name_bouquet = dat_line.substr(1, index - 1);
				string bouquet_name = dat_line.substr(index + 1);

				     if ( name_bouquet == "00" ) bq_n00 = bouquet_name;
				else if ( name_bouquet == "01" ) bq_n01 = bouquet_name;
				else if ( name_bouquet == "02" ) bq_n02 = bouquet_name;
				else if ( name_bouquet == "03" ) bq_n03 = bouquet_name;
				else if ( name_bouquet == "04" ) bq_n04 = bouquet_name;
				else if ( name_bouquet == "05" ) bq_n05 = bouquet_name;
				else if ( name_bouquet == "06" ) bq_n06 = bouquet_name;
				else if ( name_bouquet == "07" ) bq_n07 = bouquet_name;
				else if ( name_bouquet == "08" ) bq_n08 = bouquet_name;
				else if ( name_bouquet == "09" ) bq_n09 = bouquet_name;
				else if ( name_bouquet == "0a" ) bq_n0a = bouquet_name;
				else if ( name_bouquet == "0b" ) bq_n0b = bouquet_name;
				else if ( name_bouquet == "0c" ) bq_n0c = bouquet_name;
				else if ( name_bouquet == "0d" ) bq_n0d = bouquet_name;
				else if ( name_bouquet == "0e" ) bq_n0e = bouquet_name;
				else if ( name_bouquet == "0f" ) bq_n0f = bouquet_name;
				else if ( name_bouquet == "10" ) bq_n10 = bouquet_name;
				else if ( name_bouquet == "11" ) bq_n11 = bouquet_name;
				else if ( name_bouquet == "12" ) bq_n12 = bouquet_name;
				else if ( name_bouquet == "13" ) bq_n13 = bouquet_name;
				else if ( name_bouquet == "14" ) bq_n14 = bouquet_name;
				else if ( name_bouquet == "15" ) bq_n15 = bouquet_name;
				else if ( name_bouquet == "u1" ) bq_nu1 = bouquet_name;
				else if ( name_bouquet == "u2" ) bq_nu2 = bouquet_name;
				else if ( name_bouquet == "u3" ) bq_nu3 = bouquet_name;
				else if ( name_bouquet == "u4" ) bq_nu4 = bouquet_name;
				else if ( name_bouquet == "u5" ) bq_nu5 = bouquet_name;
			}
		}
		custom_name_file.close();
	}

	ofstream bq_00 ("/tmp/userbouquet.ukcvs00.tv"); bq_00 << bq_N << bq_n00 << endl << bq_S << endl << bq_D << bq_n1 << bq_n00 << bq_n2 << endl;
	ofstream bq_01 ("/tmp/userbouquet.ukcvs01.tv"); bq_01 << bq_N << bq_s1 << bq_n01 << endl << bq_S << endl << bq_D << bq_n1 << bq_n01 << bq_n2 << endl;
	ofstream bq_02 ("/tmp/userbouquet.ukcvs02.tv"); bq_02 << bq_N << bq_s1 << bq_n02 << endl << bq_S << endl << bq_D << bq_n1 << bq_n02 << bq_n2 << endl;
	ofstream bq_03 ("/tmp/userbouquet.ukcvs03.tv"); bq_03 << bq_N << bq_s1 << bq_n03 << endl << bq_S << endl << bq_D << bq_n1 << bq_n03 << bq_n2 << endl;
	ofstream bq_04 ("/tmp/userbouquet.ukcvs04.tv"); bq_04 << bq_N << bq_s1 << bq_n04 << endl << bq_S << endl << bq_D << bq_n1 << bq_n04 << bq_n2 << endl;
	ofstream bq_05 ("/tmp/userbouquet.ukcvs05.tv"); bq_05 << bq_N << bq_s1 << bq_n05 << endl << bq_S << endl << bq_D << bq_n1 << bq_n05 << bq_n2 << endl;
	ofstream bq_06 ("/tmp/userbouquet.ukcvs06.tv"); bq_06 << bq_N << bq_s1 << bq_n06 << endl << bq_S << endl << bq_D << bq_n1 << bq_n06 << bq_n2 << endl;
	ofstream bq_07 ("/tmp/userbouquet.ukcvs07.tv"); bq_07 << bq_N << bq_s1 << bq_n07 << endl << bq_S << endl << bq_D << bq_n1 << bq_n07 << bq_n2 << endl;
	ofstream bq_08 ("/tmp/userbouquet.ukcvs08.tv"); bq_08 << bq_N << bq_s1 << bq_n08 << endl << bq_S << endl << bq_D << bq_n1 << bq_n08 << bq_n2 << endl;
	ofstream bq_09 ("/tmp/userbouquet.ukcvs09.tv"); bq_09 << bq_N << bq_s1 << bq_n09 << endl << bq_S << endl << bq_D << bq_n1 << bq_n09 << bq_n2 << endl;
	ofstream bq_0a ("/tmp/userbouquet.ukcvs0a.tv"); bq_0a << bq_N << bq_s1 << bq_n0a << endl << bq_S << endl << bq_D << bq_n1 << bq_n0a << bq_n2 << endl;
	ofstream bq_0b ("/tmp/userbouquet.ukcvs0b.tv"); bq_0b << bq_N << bq_s1 << bq_n0b << endl << bq_S << endl << bq_D << bq_n1 << bq_n0b << bq_n2 << endl;
	ofstream bq_0c ("/tmp/userbouquet.ukcvs0c.tv"); bq_0c << bq_N << bq_s1 << bq_n0c << endl << bq_S << endl << bq_D << bq_n1 << bq_n0c << bq_n2 << endl;
	ofstream bq_0d ("/tmp/userbouquet.ukcvs0d.tv"); bq_0d << bq_N << bq_s1 << bq_n0d << endl << bq_S << endl << bq_D << bq_n1 << bq_n0d << bq_n2 << endl;
	ofstream bq_0e ("/tmp/userbouquet.ukcvs0e.tv"); bq_0e << bq_N << bq_s1 << bq_n0e << endl << bq_S << endl << bq_D << bq_n1 << bq_n0e << bq_n2 << endl;
	ofstream bq_0f ("/tmp/userbouquet.ukcvs0f.tv"); bq_0f << bq_N << bq_s1 << bq_n0f << endl << bq_S << endl << bq_D << bq_n1 << bq_n0f << bq_n2 << endl;
	ofstream bq_10 ("/tmp/userbouquet.ukcvs10.tv"); bq_10 << bq_N << bq_s1 << bq_n10 << endl << bq_S << endl << bq_D << bq_n1 << bq_n10 << bq_n2 << endl;
	ofstream bq_14 ("/tmp/userbouquet.ukcvs14.tv"); bq_14 << bq_N << bq_s1 << bq_n14 << endl << bq_S << endl << bq_D << bq_n1 << bq_n14 << bq_n2 << endl;
	ofstream bq_15 ("/tmp/userbouquet.ukcvs15.tv"); bq_15 << bq_N << bq_s1 << bq_n15 << endl << bq_S << endl << bq_D << bq_n1 << bq_n15 << bq_n2 << endl;
	ofstream bq_u1 ("/tmp/userbouquet.ukcvs-user1.tv"); bq_u1 << bq_N << bq_s1 << bq_nu1 << endl << bq_S << endl << bq_D << bq_n1 << bq_nu1 << bq_n2 << endl;
	ofstream bq_u2 ("/tmp/userbouquet.ukcvs-user2.tv"); bq_u2 << bq_N << bq_s1 << bq_nu2 << endl << bq_S << endl << bq_D << bq_n1 << bq_nu2 << bq_n2 << endl;
	ofstream bq_u3 ("/tmp/userbouquet.ukcvs-user3.tv"); bq_u3 << bq_N << bq_s1 << bq_nu3 << endl << bq_S << endl << bq_D << bq_n1 << bq_nu3 << bq_n2 << endl;
	ofstream bq_u4 ("/tmp/userbouquet.ukcvs-user4.tv"); bq_u4 << bq_N << bq_s1 << bq_nu4 << endl << bq_S << endl << bq_D << bq_n1 << bq_nu4 << bq_n2 << endl;
	ofstream bq_u5 ("/tmp/userbouquet.ukcvs-user5.tv"); bq_u5 << bq_N << bq_s1 << bq_nu5 << endl << bq_S << endl << bq_D << bq_n1 << bq_nu5 << bq_n2 << endl;

	bq_n00 = "28.2E UK Bouquets";
	bq_n01 = "Entertainment";
	bq_n02 = "Lifestyle and Culture";
	bq_n03 = "Movies";
	bq_n04 = "Music";
	bq_n05 = "Sports";
	bq_n06 = "News";
	bq_n07 = "Documentaries";
	bq_n08 = "Religious";
	bq_n09 = "Kids";
	bq_n0a = "Shopping";
	bq_n0b = "Sky Box Office";
	bq_n0c = "International";
	bq_n0d = "Gaming and Dating";
	bq_n0e = "Specialist";
	bq_n0f = "Adult";
	bq_n10 = "Other";
	bq_n11 = "Sky Sports Interactive";
	bq_n12 = "BT Sports Interactive";
	bq_n13 = "Sky Anytime";
	bq_n14 = "Free To Air";
	bq_n15 = "High Definition";

	if (custom_sort != 1)
	{
		for ( count = 0; count < 100; count++ )
			bq_00 << bq_P << endl << bq_d << " " << endl;
	}
	else
	{
		count = 0;
		bq_00 << bq_S << endl << bq_d << bq_n1 << bq_rgl << bq_n2 << endl;
		for( map<string, channel_t>::iterator i = TV.begin(); i != TV.end(); ++i )
		{
			count++;

			bq_00 << bq_s << hex << TV[(*i).first].type << ":" << TV[(*i).first].sid << ":" << TV[(*i).first].tsid << bq_O << TV[(*i).first].nspace << bq_F << endl;
			if (numbering)
				bq_00 << bq_d << dec << count << bq_s2 << TV[(*i).first].name << endl;
			else
				bq_00 << bq_d << dec << TV[(*i).first].name << endl;

			if (count > 4) break;
		}
		bq_00 << bq_S << endl << bq_d << bq_n1 << bq_n15 << bq_n2 << endl;
	}

	char supp_file[256];
	memset(supp_file, '\0', 256);
	sprintf(supp_file, "%s/supplement.txt", curr_path);
	ifstream supplement_file(supp_file);

	if (supplement_file.is_open())
	{
		unsigned int pre, index, skyid;
		string dat_line, epg_id;
		bool is_skyid, is_epgid, is_type, is_sid, is_tsid, is_nspace, is_provider, is_ca, is_name, is_complete, is_reassign;

		ofstream autobouquets_log;
		autobouquets_log.open ("/tmp/autobouquets.log");

		while (!supplement_file.eof() && getline(supplement_file, dat_line))
		{
			if (dat_line.length() > 0 && dat_line[0] != '#' && dat_line[0] != '$')
			{
				index = dat_line.find(':');
				channel.skyid = dat_line.substr(0, index);
				skyid = atoi(channel.skyid.c_str());
				is_skyid = (index == 0) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				epg_id = dat_line.substr(pre, index-pre);
				is_epgid = (index == pre) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				channel.type = dat_line.substr(pre, index-pre);
				is_type = (index == pre) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				channel.sid = dat_line.substr(pre, index-pre);
				is_sid = (index == pre) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				channel.tsid = dat_line.substr(pre, index-pre);
				is_tsid = (index == pre) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				channel.nspace = dat_line.substr(pre, index-pre);
				is_nspace = (index == pre) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				channel.provider = dat_line.substr(pre, index-pre);
				is_provider = (index == pre) ? false : true;

				pre = index + 1;
				index = dat_line.find(':', pre);
				channel.ca = dat_line.substr(pre, index-pre);
				is_ca = (index == pre) ? false : true;

				pre = index + 1;
				channel.name = dat_line.substr(pre);
				is_name = (dat_line.length() == pre) ? false : true;

				is_complete = (is_skyid && is_epgid && is_type && is_sid && is_tsid && is_nspace && is_provider && is_ca && is_name) ? true : false;
				is_reassign = (is_skyid && is_epgid) ? true : false;

				bool found_reassign = false;

				if (is_epgid && !is_complete)
				{
					if (DATA.find(epg_id) != DATA.end())
					{
						autobouquets_log << DATA[epg_id].skyid << ":" << epg_id << ":" << DATA[epg_id].type << ":"
						<< DATA[epg_id].sid << ":" << DATA[epg_id].tsid << ":" << DATA[epg_id].nspace << ":"
						<< DATA[epg_id].provider << ":" << DATA[epg_id].ca << ":" << DATA[epg_id].name << endl;

						if (is_type)		DATA[epg_id].type = channel.type;
						if (is_sid)		DATA[epg_id].sid = channel.sid;
						if (is_tsid)		DATA[epg_id].tsid = channel.tsid;
						if (is_nspace)		DATA[epg_id].nspace = channel.nspace;
						if (is_provider)	DATA[epg_id].provider = channel.provider;
						if (is_ca)		DATA[epg_id].ca = channel.ca;
						if (is_name)		DATA[epg_id].name = channel.name;
						if (is_reassign && !found_reassign)
						{
							autobouquets_log << "REASSIGN BAT: DATA > ";
							string ch_skyid = channel.skyid;
							channel = DATA[epg_id];
							channel.skyid = ch_skyid;
							DATA.erase(epg_id);
							found_reassign = true;
						}
						else
						{
							autobouquets_log << DATA[epg_id].skyid << ":" << epg_id << ":" << DATA[epg_id].type << ":"
							<< DATA[epg_id].sid << ":" << DATA[epg_id].tsid << ":" << DATA[epg_id].nspace << ":"
							<< DATA[epg_id].provider << ":" << DATA[epg_id].ca << ":" << DATA[epg_id].name << endl << endl;
						}
					}

					if (is_reassign)
					{
						if (TV.find(channel.skyid) != TV.end())
						{
							DATA[TV[channel.skyid].skyid] = TV[channel.skyid];
							DATA[TV[channel.skyid].skyid].skyid = "65535";
						}
						if (TEST.find(channel.skyid) != TEST.end())
						{
							DATA[TEST[channel.skyid].skyid] = TEST[channel.skyid];
							DATA[TEST[channel.skyid].skyid].skyid = "65535";
						}
					}

					for( map<string, channel_t>::iterator i = TV.begin(); i != TV.end(); ++i )
					{
						if ((*i).second.skyid == epg_id)
						{
							autobouquets_log << (*i).first  << ":" << (*i).second.skyid << ":" << (*i).second.type << ":"
							<< (*i).second.sid << ":" << (*i).second.tsid << ":" << (*i).second.nspace << ":" 
							<< (*i).second.provider << ":" << (*i).second.ca << ":" << (*i).second.name << endl;

							if (is_type)		(*i).second.type = channel.type;
							if (is_sid)		(*i).second.sid = channel.sid;
							if (is_tsid)		(*i).second.tsid = channel.tsid;
							if (is_nspace)		(*i).second.nspace = channel.nspace;
							if (is_provider)	(*i).second.provider = channel.provider;
							if (is_ca)		(*i).second.ca = channel.ca;
							if (is_name)		(*i).second.name = channel.name;
							if (is_reassign && !found_reassign)
							{
								autobouquets_log << "REASSIGN BAT: TV > ";
								TV[(*i).first].skyid = channel.skyid;
								channel = TV[(*i).first];
								TV.erase((*i).first);
								found_reassign = true;
							}
							else
							{
								autobouquets_log << (*i).first  << ":" << (*i).second.skyid << ":" << (*i).second.type << ":"
								<< (*i).second.sid << ":" << (*i).second.tsid << ":" << (*i).second.nspace << ":"
								<< (*i).second.provider << ":" << (*i).second.ca << ":" << (*i).second.name << endl << endl;
							}
						}
					}

					for( map<string, channel_t>::iterator i = TEST.begin(); i != TEST.end(); ++i )
					{
						if ((*i).second.skyid == epg_id)
						{
							autobouquets_log << (*i).first  << ":" << (*i).second.skyid << ":" << (*i).second.type << ":"
							<< (*i).second.sid << ":" << (*i).second.tsid << ":" << (*i).second.nspace << ":"
							<< (*i).second.provider << ":" << (*i).second.ca << ":" << (*i).second.name << endl;

							if (is_type)		(*i).second.type = channel.type;
							if (is_sid)		(*i).second.sid = channel.sid;
							if (is_tsid)		(*i).second.tsid = channel.tsid;
							if (is_nspace)		(*i).second.nspace = channel.nspace;
							if (is_provider)	(*i).second.provider = channel.provider;
							if (is_ca)		(*i).second.ca = channel.ca;
							if (is_name)		(*i).second.name = channel.name;
							if (is_reassign && !found_reassign)
							{
								autobouquets_log << "REASSIGN BAT: TEST > ";
								TEST[(*i).first].skyid = channel.skyid;
								channel = TEST[(*i).first];
								TEST.erase((*i).first);
								found_reassign = true;
							}
							else
							{
								autobouquets_log << (*i).first  << ":" << (*i).second.skyid << ":" << (*i).second.type << ":"
								<< (*i).second.sid << ":" << (*i).second.tsid << ":" << (*i).second.nspace << ":"
								<< (*i).second.provider << ":" << (*i).second.ca << ":" << (*i).second.name << endl << endl;
							}
						}
					}
				}

				if (is_complete || (is_reassign && found_reassign))
				{
					if (is_complete) autobouquets_log << "NEW CHANNEL: ";

					if ((skyid > 100) && (skyid < 1000))
					{
						TV[channel.skyid] = channel;
						TV[channel.skyid].skyid = epg_id;
						autobouquets_log << "TV" << endl;
					}
					else if ((skyid > 3100) && (skyid < 4000))
					{
						RADIO[channel.skyid] = channel;
						RADIO[channel.skyid].skyid = epg_id;
						autobouquets_log << "RADIO" << endl;
					}
					else if (skyid == 0xffff)
					{
						DATA[epg_id] = channel;
						autobouquets_log << "DATA" << endl;
					}
					else
					{
						TEST[channel.skyid] = channel;
						TEST[channel.skyid].skyid = epg_id;
						autobouquets_log << "TEST" << endl;
					}

					autobouquets_log << channel.skyid << ":" << epg_id << ":" << channel.type << ":"
					<< channel.sid << ":" << channel.tsid << ":" << channel.nspace << ":"
					<< channel.provider << ":" << channel.ca << ":" << channel.name << endl << endl;
				}
			}
		}
		autobouquets_log.close();
	}
	supplement_file.close();

	if (custom_swap)
	{
		char swap_file[256];
		memset(swap_file, '\0', 256);
		sprintf(swap_file, "%s/custom_swap.txt", curr_path);
		ifstream custom_swap_file(swap_file);

		if (custom_swap_file.is_open())
		{
			string dat_line;
			while (!custom_swap_file.eof() && getline(custom_swap_file, dat_line))
			{
				if (dat_line.length() > 0 && dat_line[0] != '#')
				{
					bool swap_found = true;
					int index = dat_line.find('=');

					string swap1 = dat_line.substr(0, index);
					string swap2 = dat_line.substr(index + 1);
					int swap1_pos = atoi(swap1.c_str());
					int swap2_pos = atoi(swap2.c_str());

					if (swap1 != swap2)
					{
						if ((swap1_pos > 100) && (swap1_pos < 1000))
						{
							if (TV.find(swap1) != TV.end())
								channel = TV[swap1];
							else
								swap_found = false;
						}
						else
						{
							if (TEST.find(swap1) != TEST.end())
								channel = TEST[swap1];
							else
								swap_found = false;
						}

						if ((swap2_pos > 100) && (swap2_pos < 1000))
						{
							if (TV.find(swap2) != TV.end())
							{
								if ((swap1_pos > 100) && (swap1_pos < 1000))
									TV[swap1] = TV[swap2];
								else
									TEST[swap1] = TV[swap2];
							}
						}
						else
						{
							if (TEST.find(swap2) != TEST.end())
							{
								if ((swap1_pos > 100) && (swap1_pos < 1000))
									TV[swap1] = TEST[swap2];
								else
									TEST[swap1] = TEST[swap2];
							}
						}

						if (swap_found)
						{
							if ((swap2_pos > 100) && (swap2_pos < 1000))
								TV[swap2] = channel;
							else
								TEST[swap2] = channel;
						}
					}
				}
			}

			custom_swap_file.close();
		}
	}

	if (custom_sort > 1)
	{
		char sort_file[256];
		memset(sort_file, '\0', 256);
		sprintf(sort_file, "%s/custom_sort_%i.txt", curr_path, custom_sort-1);
		ifstream custom_sort_file(sort_file);

		if (custom_sort_file.is_open())
		{
			string dat_line;
			while (!custom_sort_file.eof() && getline(custom_sort_file, dat_line))
			{
				if (dat_line.length() > 0 && dat_line[0] != '#' && dat_line[0] != '$')
				{
					bool channel_found = false;
					int index = dat_line.find('=');
					string sort_bouquet = dat_line.substr(0, index);
					string sort_skyid = dat_line.substr(index + 1);
					int skyid = atoi(sort_skyid.c_str());
					
					if ( TV.count(sort_skyid) > 0 )
					{
						channel = TV[sort_skyid];
						channel_found = true;
					}
					else if ( TEST.count(sort_skyid) > 0 )
					{
						channel = TEST[sort_skyid];
						channel_found = true;
					}
					else if ( RADIO.count(sort_skyid) > 0 )
					{
						channel = RADIO[sort_skyid];
						channel_found = true;
					}
					else if ( skyid > 0xffff )
					{
						sort_skyid = to_string<int>((skyid - 0xffff), dec);
						if ( DATA.count(sort_skyid) > 0 )
						{
							channel = DATA[sort_skyid];
							channel_found = true;
						}
					}

					if (channel_found)
					{
						     if ( sort_bouquet == "01" ) write_bouquet_service(channel,bq_01,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "02" ) write_bouquet_service(channel,bq_02,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "03" ) write_bouquet_service(channel,bq_03,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "04" ) write_bouquet_service(channel,bq_04,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "05" ) write_bouquet_service(channel,bq_05,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "06" ) write_bouquet_service(channel,bq_06,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "07" ) write_bouquet_service(channel,bq_07,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "08" ) write_bouquet_service(channel,bq_08,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "09" ) write_bouquet_service(channel,bq_09,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "0a" ) write_bouquet_service(channel,bq_0a,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "0b" ) write_bouquet_service(channel,bq_0b,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "0c" ) write_bouquet_service(channel,bq_0c,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "0d" ) write_bouquet_service(channel,bq_0d,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "0e" ) write_bouquet_service(channel,bq_0e,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "0f" ) write_bouquet_service(channel,bq_0f,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "10" ) write_bouquet_service(channel,bq_10,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "u1" ) write_bouquet_service(channel,bq_u1,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "u2" ) write_bouquet_service(channel,bq_u2,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "u3" ) write_bouquet_service(channel,bq_u3,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "u4" ) write_bouquet_service(channel,bq_u4,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
						else if ( sort_bouquet == "u5" ) write_bouquet_service(channel,bq_u5,numbering,sort_skyid,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
					}
				}
			}
			custom_sort_file.close();
		}
	}

	bq_u1.close(); bq_u2.close(); bq_u3.close(); bq_u4.close(); bq_u5.close();

	count = 100;
	unsigned short hdcount = 0;
	string skynum = "";

	for( map<string, channel_t>::iterator i = TV.begin(); i != TV.end(); ++i )
	{
		count++;
		unsigned short skyid = atoi((*i).first.c_str());
		bool parentalguidance = false;

		if (count == skyid)
			write_bouquet_names(count,bq_n01,bq_n02,bq_n03,bq_n04,bq_n05,bq_n06,bq_n07,bq_n08,bq_n09,bq_n0a,bq_n0b,
				bq_n0c,bq_n0d,bq_n0e,bq_n0f,bq_n10,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2,parentalcontrol);
		else
		{	// placeholder
			while (count < skyid)
			{
				count++;

				if (custom_sort == 1)
				{
					if ( count > 100 && count <= 301 ) bq_01 << bq_P << endl << bq_d << " " << endl;
				//	else if ( count > 239 && count <= 240 ) bq_02 << bq_P << endl << bq_d << " " << endl; //Reserved: Lifestyle and Culture
					else if ( count > 300 && count <= 350 ) bq_03 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 349 && count <= 401 ) bq_04 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 400 && count <= 501 ) bq_05 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 500 && count <= 520 ) bq_06 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 519 && count <= 580 ) bq_07 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 579 && count <= 601 ) bq_08 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 600 && count <= 650 ) bq_09 << bq_P << endl << bq_d << " " << endl;
					else if ( count > 649 && count <= 700 ) bq_0a << bq_P << endl << bq_d << " " << endl;
					else if ( count > 699 && count <= 780 ) bq_0b << bq_P << endl << bq_d << " " << endl;
					else if ( count > 779 && count <= 871 ) bq_0c << bq_P << endl << bq_d << " " << endl;
					else if ( count > 870 && count <= 881 ) 
					{
						// add placholder padding to prevous bq - gaming and dating
						if (parentalcontrol)
							bq_0c << bq_P << endl << bq_d << " " << endl;
						else
							bq_0d << bq_P << endl << bq_d << " " << endl;
					}
					else if ( count > 880 && count <= 900 ) bq_0e << bq_P << endl << bq_d << " " << endl;
					else if ( count > 899 && count <= 950 ) 
					{
						// add placholder padding to prevous bq - adult
						if (parentalcontrol)
							bq_0e << bq_P << endl << bq_d << " " << endl;
						else
							bq_0f << bq_P << endl << bq_d << " " << endl;
					}
					else bq_10 << bq_P << endl << bq_d << " " << endl;
				}
				else
					bq_00 << bq_P << endl << bq_d << " " << endl;

				write_bouquet_names(count,bq_n01,bq_n02,bq_n03,bq_n04,bq_n05,bq_n06,bq_n07,bq_n08,bq_n09,bq_n0a,bq_n0b,
					bq_n0c,bq_n0d,bq_n0e,bq_n0f,bq_n10,bq_00,bq_14,bq_15,custom_sort,bq_S,bq_d,bq_n1,bq_n2,parentalcontrol);

			}
			count = skyid;
		}

		parentalguidance = (( skyid > 870 && skyid < 881 ) || ( skyid > 899 && skyid < 950 )) ? true : false;

		if (custom_sort != 1)
		{
			if (parentalguidance && parentalcontrol)
				bq_00 << bq_P << endl << bq_d << " " << endl;
			else
				write_bouquet_service((*i).second,bq_00,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
		}

		if (custom_sort < 2)
		{
			     if ( skyid > 100 && skyid < 301 ) write_bouquet_service((*i).second,bq_01,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
		//	else if ( skyid > 239 && skyid < 240 ) write_bouquet_service((*i).second,bq_02,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P); //Reserved: Lifestyle and Culture
			else if ( skyid > 300 && skyid < 350 ) write_bouquet_service((*i).second,bq_03,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 349 && skyid < 401 ) write_bouquet_service((*i).second,bq_04,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 400 && skyid < 501 ) write_bouquet_service((*i).second,bq_05,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 500 && skyid < 520 ) write_bouquet_service((*i).second,bq_06,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 519 && skyid < 580 ) write_bouquet_service((*i).second,bq_07,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 579 && skyid < 601 ) write_bouquet_service((*i).second,bq_08,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 600 && skyid < 650 ) write_bouquet_service((*i).second,bq_09,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 649 && skyid < 700 ) write_bouquet_service((*i).second,bq_0a,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 699 && skyid < 780 ) write_bouquet_service((*i).second,bq_0b,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 779 && skyid < 871 ) write_bouquet_service((*i).second,bq_0c,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 870 && skyid < 881 )
			{
				// add placholder padding to prevous bq - gaming and dating
				if (parentalcontrol)
				{
					if (custom_sort == 1)
						bq_0c << bq_P << endl << bq_d << " " << endl;
				}
				else
 					write_bouquet_service((*i).second,bq_0d,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			}
			else if ( skyid > 880 && skyid < 900 ) write_bouquet_service((*i).second,bq_0e,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			else if ( skyid > 899 && skyid < 950 )
			{
				// add placholder padding to prevous bq - adult
				if (parentalcontrol)
				{
					if (custom_sort == 1)
						bq_0e << bq_P << endl << bq_d << " " << endl;
				}
				else
					write_bouquet_service((*i).second,bq_0f,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			}
			else
 				write_bouquet_service((*i).second,bq_10,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
		}

		if ( ((*i).second.ca == "FTA") && !(parentalguidance && parentalcontrol) )
 			write_bouquet_service((*i).second,bq_14,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);

		int is_type = atoi((*i).second.type.c_str());
		if ( ( is_type == 19 || is_type == 87 ) && !(parentalguidance && parentalcontrol) )
		{
 			write_bouquet_service((*i).second,bq_15,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);

			if (custom_sort == 1)
			{
				hdcount++;
				if (hdcount <= 95) // up to max 100 hd channels - first original 5 local region
 					write_bouquet_service((*i).second,bq_00,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			}
		}

		if (piconstyle != 0)
 		{
 			char picon_source[256]; memset(picon_source, '\0', 256);
 			char picon_target[256]; memset(picon_target, '\0', 256);
			if (piconstyle == 1)
			{
				if (SYMLINK_NAME((*i).second.nspace.c_str()).size() > 0)
				{
					string picon_sid  = (*i).second.sid;  stringToUpper(picon_sid);
					string picon_tsid = (*i).second.tsid; stringToUpper(picon_tsid);
					string picon_type = (*i).second.type; stringToUpper(picon_type);
					string picon_nspace = (*i).second.nspace; stringToUpper(picon_nspace);
					sprintf(picon_source, "%s1_0_%s_%s_%s_2_%s_0_0_0.png", picon_link.c_str(), picon_type.c_str(), picon_sid.c_str(), picon_tsid.c_str(), picon_nspace.c_str());
					sprintf(picon_target, "%s282E_%s.png", picon_folder.c_str(), (*i).second.skyid.c_str());
					symlink(picon_target, picon_source);
				}
			}
			else
			{
				string picon_name = SYMLINK_NAME((*i).second.name.c_str());
				if (picon_name.size() > 0)
				{
					sprintf(picon_source, "%s%s%s.png", picon_link.c_str(), (*i).first.c_str(), picon_name.c_str());
					sprintf(picon_target, "%s%s.png", picon_folder.c_str(), picon_name.c_str());
					symlink(picon_target, picon_source);
				}
			}
		}

		database_csv << dec << skyid << "," << dec << (*i).second.skyid << ",0x" << hex << (*i).second.type << ",0x" << hex << (*i).second.sid;
		database_csv << ",0x" << hex << (*i).second.tsid << ",\"" << (*i).second.ca << "\",\"" << (*i).second.name << "\"";
		database_csv << ",1:0:" << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
	}

	TV.clear();

	if (custom_sort == 1)
	{
		// placholder channel padding
		// up to max 100 hd channels - first original 5 local region
		for ( ; hdcount < 95; hdcount++ )
			bq_00 << bq_P << endl << bq_d << " " << endl;
	}

	bq_01.close(); bq_02.close(); bq_03.close(); bq_04.close(); bq_05.close();
	bq_06.close(); bq_07.close(); bq_08.close(); bq_09.close(); bq_0a.close();
	bq_0b.close(); bq_0c.close(); bq_0d.close(); bq_0e.close(); bq_0f.close();

	ofstream bq_11; ofstream bq_12; ofstream bq_13;

	bq_11.open ("/tmp/userbouquet.ukcvs11.tv");
	bq_12.open ("/tmp/userbouquet.ukcvs12.tv");
	bq_13.open ("/tmp/userbouquet.ukcvs13.tv");

	bq_11 << bq_N << bq_s1 << bq_n11 << endl << bq_S << endl << bq_D << bq_n1 << bq_n11 << bq_n2 << endl;
	bq_12 << bq_N << bq_s1 << bq_n12 << endl << bq_S << endl << bq_D << bq_n1 << bq_n12 << bq_n2 << endl;
	bq_13 << bq_N << bq_s1 << bq_n13 << endl << bq_S << endl << bq_D << bq_n1 << bq_n13 << bq_n2 << endl;

	if (extra)
	{
		bq_10 << bq_S << endl << bq_d << bq_n1 << bq_tst << bq_n2 << endl;

		for( map<string, channel_t>::iterator i = TEST.begin(); i != TEST.end(); ++i )
		{
			unsigned short skyid = atoi((*i).first.c_str());
			if (skyid < 1000)
 				write_bouquet_service((*i).second,bq_10,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
		}
	}

	for( map<string, channel_t>::iterator i = TEST.begin(); i != TEST.end(); ++i )
	{
		unsigned short skyid = atoi((*i).first.c_str());
		if (skyid > 999)
		{
			if (extra)
 				write_bouquet_service((*i).second,bq_10,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
		}

		if (piconstyle != 0)
 		{
 			char picon_source[256]; memset(picon_source, '\0', 256);
 			char picon_target[256]; memset(picon_target, '\0', 256);
			if (piconstyle == 1)
			{
				if (SYMLINK_NAME((*i).second.nspace.c_str()).size() > 0)
				{
					string picon_sid  = (*i).second.sid;  stringToUpper(picon_sid);
					string picon_tsid = (*i).second.tsid; stringToUpper(picon_tsid);
					string picon_type = (*i).second.type; stringToUpper(picon_type);
					string picon_nspace = (*i).second.nspace; stringToUpper(picon_nspace);
					sprintf(picon_source, "%s1_0_%s_%s_%s_2_%s_0_0_0.png", picon_link.c_str(), picon_type.c_str(), picon_sid.c_str(), picon_tsid.c_str(), picon_nspace.c_str());
					sprintf(picon_target, "%s282E_%s.png", picon_folder.c_str(), (*i).second.skyid.c_str());
					symlink(picon_target, picon_source);
				}
			}
			else
			{
				string picon_name = SYMLINK_NAME((*i).second.name.c_str());
				if (picon_name.size() > 0)
				{
					sprintf(picon_source, "%s%s%s.png", picon_link.c_str(), (*i).first.c_str(), picon_name.c_str());
					sprintf(picon_target, "%s%s.png", picon_folder.c_str(), picon_name.c_str());
					symlink(picon_target, picon_source);
				}
			}
		}

		database_csv << dec << (*i).first << "," << dec << (*i).second.skyid << ",0x" << hex << (*i).second.type << ",0x" << hex << (*i).second.sid;
		database_csv << ",0x" << hex << (*i).second.tsid << ",\"" << (*i).second.ca << "\",\"" << (*i).second.name << "\"";
		database_csv << ",1:0:" << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
	}

	TEST.clear();

	if (extra)
		bq_10 << bq_S << endl << bq_d << bq_n1 << bq_dat << bq_n2 << endl;

	for( map<string, channel_t>::iterator i = DATA.begin(); i != DATA.end(); ++i )
	{
		count++;
		unsigned short skyid = atoi((*i).first.c_str());

		if ( skyid > (ssa1 -1) && skyid < ssa2 )
		{
			if (skyid == ssa1) {
				bq_14 << bq_S << endl << bq_d << bq_n1 << bq_n11 << bq_n2 << endl;
				bq_15 << bq_S << endl << bq_d << bq_n1 << bq_n11 << bq_n2 << endl;
				if (custom_sort != 1) bq_00 << bq_S << endl << bq_d << bq_n1 << bq_n11 << bq_n2 << endl;
			}

			if (custom_sort != 1)
			{
				bq_00 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_00 << bq_d << dec << (*i).second.name << endl;
			}

			bq_11 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
			bq_11 << bq_d << dec << (*i).second.name << endl;

			if ( (*i).second.ca == "FTA" )
			{
				bq_14 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_14 << bq_d << (*i).second.name << endl;
			}

			int is_type = atoi((*i).second.type.c_str());
			if ( is_type == 19 || is_type == 87 )
			{
				bq_15 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_15 << bq_d << dec << (*i).second.name << endl;
			}
		}
		else if (( skyid > (btx1 - 1) && skyid < btx2 ) || ( skyid > btx3 && skyid < btx4 ))
		{
			if (skyid == btx1) {
				bq_14 << bq_S << endl << bq_d << bq_n1 << bq_n12 << bq_n2 << endl;
				bq_15 << bq_S << endl << bq_d << bq_n1 << bq_n12 << bq_n2 << endl;
				if (custom_sort != 1) bq_00 << bq_S << endl << bq_d << bq_n1 << bq_n12 << bq_n2 << endl;
			}

			if (custom_sort != 1)
			{
				bq_00 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_00 << bq_d << dec << (*i).second.name << endl;
			}

			bq_12 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
			bq_12 << bq_d << dec << (*i).second.name << endl;

			if ( (*i).second.ca == "FTA" )
			{
				bq_14 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_14 << bq_d << dec << (*i).second.name << endl;
			}

			int is_type = atoi((*i).second.type.c_str());
			if ( is_type == 19 || is_type == 87 )
			{
				bq_15 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_15 << bq_d << dec << (*i).second.name << endl;
			}
		}
		else if ( skyid > (any1 - 1) && skyid < any2 )
		{
			if (skyid == any1) {
				bq_14 << bq_S << endl << bq_d << bq_n1 << bq_n13 << bq_n2 << endl;
				bq_15 << bq_S << endl << bq_d << bq_n1 << bq_n13 << bq_n2 << endl;
				if (custom_sort != 1) bq_00 << bq_S << endl << bq_d << bq_n1 << bq_n13 << bq_n2 << endl;
			}

			if (custom_sort != 1)
			{
				bq_00 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_00 << bq_d << dec << (*i).second.name << endl;
			}

			bq_13 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
			bq_13 << bq_d << dec << (*i).second.name << endl;

			if ( (*i).second.ca == "FTA" )
			{
				bq_14 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_14 << bq_d << dec << (*i).second.name << endl;
			}

			int is_type = atoi((*i).second.type.c_str());
			if ( is_type == 19 || is_type == 87 )
			{
				bq_15 << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
				bq_15 << bq_d << dec << (*i).second.name << endl;
			}
		}
		else
		{	// write remainder of unassigned services to 'Other' bouquet if extra option is enabled
			if (extra)
			{
				if ((atoi((*i).second.type.c_str()) != 4 && parentalcontrol) || !parentalcontrol)
 					write_bouquet_service((*i).second,bq_10,numbering,(*i).first,bq_O,bq_F,bq_d,bq_s,bq_s2,bq_P);
			}
		}

		if (piconstyle != 0)
 		{
 			char picon_source[256]; memset(picon_source, '\0', 256);
 			char picon_target[256]; memset(picon_target, '\0', 256);
			if (piconstyle == 1)
			{
				if (SYMLINK_NAME((*i).second.nspace.c_str()).size() > 0)
				{
					string picon_sid  = (*i).second.sid;  stringToUpper(picon_sid);
					string picon_tsid = (*i).second.tsid; stringToUpper(picon_tsid);
					string picon_type = (*i).second.type; stringToUpper(picon_type);
					string picon_nspace = (*i).second.nspace; stringToUpper(picon_nspace);
					sprintf(picon_source, "%s1_0_%s_%s_%s_2_%s_0_0_0.png", picon_link.c_str(), picon_type.c_str(), picon_sid.c_str(), picon_tsid.c_str(), picon_nspace.c_str());
					sprintf(picon_target, "%s282E_%s.png", picon_folder.c_str(), (*i).first.c_str());
					symlink(picon_target, picon_source);
				}
			}
			else
			{
				string picon_name = SYMLINK_NAME((*i).second.name.c_str());
				if (picon_name.size() > 0)
				{
					sprintf(picon_source, "%s%s%s.png", picon_link.c_str(), (*i).second.skyid.c_str(), picon_name.c_str());
					sprintf(picon_target, "%s%s.png", picon_folder.c_str(), picon_name.c_str());
					symlink(picon_target, picon_source);
				}
			}
		}

		database_csv << dec << (*i).second.skyid << "," << dec << (*i).first << ",0x" << hex << (*i).second.type << ",0x" << hex << (*i).second.sid;
		database_csv << ",0x" << hex << (*i).second.tsid << ",\"" << (*i).second.ca << "\",\"" << (*i).second.name << "\"";
		database_csv << ",1:0:" << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
	}

	bq_00.close(); bq_10.close(); bq_11.close(); bq_12.close(); bq_13.close(); bq_14.close(); bq_15.close();

	ofstream bq_radio;
	bq_radio.open ("/tmp/userbouquet.ukcvs00.radio");
	bq_radio << bq_N << bq_s1 << bq_nradio << endl << bq_S << endl << bq_D << bq_n1 << bq_nradio << bq_n2 << endl;

	DATA.clear();

	for( map<string, channel_t>::iterator i = RADIO.begin(); i != RADIO.end(); ++i )
	{
		if ( atoi((*i).second.type.c_str()) == 2 )
		{
			bq_radio << bq_s << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
			bq_radio << bq_d << dec << (*i).second.name << endl;
		}

		if (piconstyle != 0)
 		{
 			char picon_source[256]; memset(picon_source, '\0', 256);
 			char picon_target[256]; memset(picon_target, '\0', 256);
			if (piconstyle == 1)
			{
				if (SYMLINK_NAME((*i).second.nspace.c_str()).size() > 0)
				{
					string picon_sid  = (*i).second.sid;  stringToUpper(picon_sid);
					string picon_tsid = (*i).second.tsid; stringToUpper(picon_tsid);
					string picon_type = (*i).second.type; stringToUpper(picon_type);
					string picon_nspace = (*i).second.nspace; stringToUpper(picon_nspace);
					sprintf(picon_source, "%s1_0_%s_%s_%s_2_%s_0_0_0.png", picon_link.c_str(), picon_type.c_str(), picon_sid.c_str(), picon_tsid.c_str(), picon_nspace.c_str());
					sprintf(picon_target, "%s282E_%s.png", picon_folder.c_str(), (*i).second.skyid.c_str());
					symlink(picon_target, picon_source);
				}
			}
			else
			{
				string picon_name = SYMLINK_NAME((*i).second.name.c_str());
				if (picon_name.size() > 0)
				{
					sprintf(picon_source, "%s%s%s.png", picon_link.c_str(), (*i).first.c_str(), picon_name.c_str());
					sprintf(picon_target, "%s%s.png", picon_folder.c_str(), picon_name.c_str());
					symlink(picon_target, picon_source);
				}
			}
		}

		database_csv << dec << (*i).first << "," << dec << (*i).second.skyid << ",0x" << hex << (*i).second.type << ",0x" << hex << (*i).second.sid;
		database_csv << ",0x" << hex << (*i).second.tsid << ",\"" << (*i).second.ca << "\",\"" << (*i).second.name << "\"";
		database_csv << ",1:0:" << hex << (*i).second.type << ":" << (*i).second.sid << ":" << (*i).second.tsid << bq_O << (*i).second.nspace << bq_F << endl;
	}

	bq_radio.close();
	database_csv.close();

	RADIO.clear();

	ofstream bouquets_tv;
	bouquets_tv.open ("/tmp/bouquets.tv");
	bouquets_tv << "#NAME UKCVS" << endl;
	if (bouquet_has_service("00")) bouquets_tv << bouquets_ntv1 << "00" << bouquets_ntv2 << endl;
	if (bouquet_has_service("01")) bouquets_tv << bouquets_ntv1 << "01" << bouquets_ntv2 << endl;
	if (bouquet_has_service("02")) bouquets_tv << bouquets_ntv1 << "02" << bouquets_ntv2 << endl;
	if (bouquet_has_service("03")) bouquets_tv << bouquets_ntv1 << "03" << bouquets_ntv2 << endl;
	if (bouquet_has_service("04")) bouquets_tv << bouquets_ntv1 << "04" << bouquets_ntv2 << endl;
	if (bouquet_has_service("05")) bouquets_tv << bouquets_ntv1 << "05" << bouquets_ntv2 << endl;
	if (bouquet_has_service("06")) bouquets_tv << bouquets_ntv1 << "06" << bouquets_ntv2 << endl;
	if (bouquet_has_service("07")) bouquets_tv << bouquets_ntv1 << "07" << bouquets_ntv2 << endl;
	if (bouquet_has_service("08")) bouquets_tv << bouquets_ntv1 << "08" << bouquets_ntv2 << endl;
	if (bouquet_has_service("09")) bouquets_tv << bouquets_ntv1 << "09" << bouquets_ntv2 << endl;
	if (bouquet_has_service("0a")) bouquets_tv << bouquets_ntv1 << "0a" << bouquets_ntv2 << endl;
	if (bouquet_has_service("0b")) bouquets_tv << bouquets_ntv1 << "0b" << bouquets_ntv2 << endl;
	if (bouquet_has_service("0c")) bouquets_tv << bouquets_ntv1 << "0c" << bouquets_ntv2 << endl;

	if (!parentalcontrol)
		if (bouquet_has_service("0d")) bouquets_tv << bouquets_ntv1 << "0d" << bouquets_ntv2 << endl;

	if (bouquet_has_service("0e")) bouquets_tv << bouquets_ntv1 << "0e" << bouquets_ntv2 << endl;

	if (!parentalcontrol)
		if (bouquet_has_service("0f")) bouquets_tv << bouquets_ntv1 << "0f" << bouquets_ntv2 << endl;

	if (bouquet_has_service("10")) bouquets_tv << bouquets_ntv1 << "10" << bouquets_ntv2 << endl;
	if (bouquet_has_service("11")) bouquets_tv << bouquets_ntv1 << "11" << bouquets_ntv2 << endl;
	if (bouquet_has_service("12")) bouquets_tv << bouquets_ntv1 << "12" << bouquets_ntv2 << endl;
	if (bouquet_has_service("13")) bouquets_tv << bouquets_ntv1 << "13" << bouquets_ntv2 << endl;
	if (bouquet_has_service("14")) bouquets_tv << bouquets_ntv1 << "14" << bouquets_ntv2 << endl;
	if (bouquet_has_service("15")) bouquets_tv << bouquets_ntv1 << "15" << bouquets_ntv2 << endl;
	if (bouquet_has_service("-user1")) bouquets_tv << bouquets_ntv1 << "-user1" << bouquets_ntv2 << endl;
	if (bouquet_has_service("-user2")) bouquets_tv << bouquets_ntv1 << "-user2" << bouquets_ntv2 << endl;
	if (bouquet_has_service("-user3")) bouquets_tv << bouquets_ntv1 << "-user3" << bouquets_ntv2 << endl;
	if (bouquet_has_service("-user4")) bouquets_tv << bouquets_ntv1 << "-user4" << bouquets_ntv2 << endl;
	if (bouquet_has_service("-user5")) bouquets_tv << bouquets_ntv1 << "-user5" << bouquets_ntv2 << endl;

	bouquets_tv << bouquets_ntv1 << "16" << bouquets_ntv2 << endl;
	bouquets_tv << "#SERVICE 1:7:2:0:0:0:0:0:0:0:FROM BOUQUET \"userbouquet.favourites.tv\" ORDER BY bouquet" << endl;
	bouquets_tv.close();

	ofstream bouquets_radio;
	bouquets_radio.open ("/tmp/bouquets.radio");
	bouquets_radio << "#NAME UKCVS" << endl;
	bouquets_radio << "#SERVICE 1:7:2:0:0:0:0:0:0:0:FROM BOUQUET \"userbouquet.ukcvs00.radio\" ORDER BY bouquet" << endl;
	bouquets_radio << "#SERVICE 1:7:2:0:0:0:0:0:0:0:FROM BOUQUET \"userbouquet.favourites.radio\" ORDER BY bouquet" << endl;
	bouquets_radio.close();

	return 0;
}

