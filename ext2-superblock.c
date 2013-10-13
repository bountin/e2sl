#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

/* for struct ext2_super_block, EXT2_SUPER_MAGIC and EXT2_BLOCK_SIZE */
#include <linux/ext2_fs.h>

#define BUFSIZE 512
#define CLEAR \
	if (clear) { printf("\r"); clear = 0; }

static unsigned int heads = 240;
static unsigned int sectors_per_head = 63;

static char *block_verbose(unsigned long long i)
{
	static char buf[512];

	sprintf(buf, "cyl %llu, head %llu, sector %llu", 
			i/(heads*sectors_per_head),
			(i / sectors_per_head) % heads, 
			i % sectors_per_head);
	return buf;
}

static int work(const char *device, unsigned long long block, int freq)
{
	char buf[BUFSIZE];
	int fd;
	struct ext2_super_block *blck = (struct ext2_super_block *) buf;	
	int n;
	int clear = 0;
	off_t offset;
	
	fd = open(device, O_RDONLY);

	if (fd == -1) {
		fprintf(stderr, "cannot open %s: ", device);
		perror("open");
		return (-1);
	}

	offset = lseek(fd, block * 512, SEEK_SET);

	if (offset == (off_t) -1) {
		perror("lseek");
		return (-1);
	}

	printf("seeked to offset %lld, wanted offset %llu (block %llu)\n", 
			offset, block * 512, block);

	while (1) {
		n = read(fd, buf, BUFSIZE);
		if (n == BUFSIZE && blck->s_magic == EXT2_SUPER_MAGIC) {
			unsigned long blck_size = EXT2_BLOCK_SIZE(blck);

			/* filter out pseudo superblocks */
			if (blck_size >= 1024 && blck_size <= 32768) {
				CLEAR;
				printf("Found: block %llu (%s), %u blocks, %u free blocks, "
						"%u block size, %s\n", 
						block, 
						block_verbose(block), 
						blck->s_blocks_count, 
						blck->s_free_blocks_count, 
						EXT2_BLOCK_SIZE(blck), 
						ctime((const time_t *) &blck->s_mtime));
			}
		} else if (n <= 0) {
			perror("read");
			return (-1);
		}
		block++;
		if ((block % freq) == 0) {
			CLEAR;	
			printf("block %llu (%s)", block, block_verbose(block));
			clear = 1;
			fflush(stdout);
		}
	}

	return (0);
}

static void usage(void)
{
	printf("Ext2 Superblock Locator, version " VERSION
			"\n"
			"\n  Options:"
			"\n    -d device         Select which device to open [/dev/hda]"
			"\n    -b block          Which block to start with [0]"
			"\n    -f n              Update status every n blocks [5000]"
			"\n"
			"\nPlease email bug reports to sascha@schumann.cx"
			"\n"
			"\n"
			);
}

int main(int argc, char **argv) 
{
	int c;
	int ret = 0;
	char *device = strdup("/dev/hda");
	unsigned long long block = 0;
	int freq = 5000;
	
	while ((c = getopt(argc, argv, "d:b:f:h")) != EOF) {
		switch(c) {
		case 'd':
			free(device);
			device = strdup(optarg);
			break;
		case 'b':
			block = strtoul(optarg, NULL, 10);
			break;
		case 'f':
			freq = strtol(optarg, NULL, 10);
			break;
		default:
			usage();
			exit(1);
		}
	}

	if (work(device, block, freq))
		ret = 1;

	free(device);
	exit(ret);
}
