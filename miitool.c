/*
 * Copyright (c) 2017, Kontron Europe GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int fd;

static int mdio_read(const char *ifname, uint16_t reg, uint16_t *val)
{
	int ret;
	struct ifreq ifr;
	uint16_t *data = (uint16_t*)&ifr.ifr_data;

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
	data[1] = reg;

	ret = ioctl(fd, SIOCGMIIPHY, &ifr);
	if (ret != 0) {
		printf("ioctl() failed: %s (%d)\n", strerror(errno), errno);
		return -1;
	}

	*val = data[3];

	return 0;
}

static int mdio_write(const char *ifname, uint16_t reg, uint16_t val)
{
	int ret;
	struct ifreq ifr;
	uint16_t *data = (uint16_t*)&ifr.ifr_data;

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
	ret = ioctl(fd, SIOCGMIIPHY, &ifr);
	if (ret != 0) {
		printf("ioctl() failed: %s (%d)\n", strerror(errno), errno);
		return -1;
	}

	data[1] = reg;
	data[2] = val;

	ret = ioctl(fd, SIOCSMIIREG, &ifr);
	if (ret != 0) {
		printf("ioctl() failed: %s (%d)\n", strerror(errno), errno);
		return -1;
	}

	return 0;
}

static void usage(const char *prog)
{
	printf("%s r[ead] <ifname> <addr>\n", prog);
	printf("%s w[rite] <ifname> <addr> <data>\n", prog);
}

int main(int argc, char **argv)
{
	int ret;
	uint16_t val;
	uint16_t addr;
	const char *ifname;

	if (argc < 4) {
		usage(argv[0]);
		return 1;
	}

	ifname = argv[2];
	addr = strtoul(argv[3], NULL, 0);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		printf("socket() failed: %s (%d)\n", strerror(errno), errno);
		return 1;
	}

	switch (argv[1][0]) {
	case 'w':
		if (argc < 5) {
			usage(argv[0]);
			return 1;
		}
		val = strtoul(argv[4], NULL, 0);
		ret = mdio_write(ifname, addr, val);
		/* fall through */
	case 'r':
		ret = mdio_read(ifname, addr, &val);
		if (ret < 0) {
			return 1;
		}
		printf("%04x\n", val);
		break;
	default:
		usage(argv[0]);
		return 1;
	}

	close(fd);

	return 0;
}
