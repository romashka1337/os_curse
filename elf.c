#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	char *ls = NULL;
	int fd = open(argv[1], O_RDONLY);
	struct stat stat = {0};

	fstat(fd, &stat);

	ls = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	close(fd);

	if (
		(unsigned char)ls[EI_MAG0] != 0x7f ||
		(unsigned char)ls[EI_MAG1] != 'E' ||
		(unsigned char)ls[EI_MAG2] != 'L' ||
		(unsigned char)ls[EI_MAG3] != 'F'
	) return -1;

	if ((unsigned char)ls[EI_CLASS] == ELFCLASS64) 
		printf("Class:\t\t\t\tELF64\n");

	if ((unsigned char)ls[EI_DATA] == ELFDATA2LSB) 
		printf("Compiled for:\t\t\tlittle-endian\n");

	if ((unsigned char)ls[EI_OSABI] == ELFOSABI_SYSV) 
		printf("OS/ABI:\t\t\t\tSYSV\n");

	if ((unsigned char)((Elf64_Ehdr *)ls)->e_type == ET_REL) 
		printf("Type:\t\t\t\tREL\n");
	
	if ((unsigned char)((Elf64_Ehdr *)ls)->e_type == ET_DYN) 
		printf("Type:\t\t\t\tDYN\n");

	printf("Entry point address:\t\t%lu\n", ((Elf64_Ehdr *)ls)->e_entry);

	Elf64_Ehdr *eh = (Elf64_Ehdr *)ls;

	const char *load_addr = NULL;
	uint32_t load_offset = 0;
	for (int i = 0; i < eh->e_phnum; i++) {
		Elf64_Phdr *ph = (Elf64_Phdr *)((char *)ls + (eh->e_phoff + eh->e_phentsize * i));
		if (ph->p_type == PT_LOAD) {
			load_addr = (const char *)ph->p_vaddr;
			load_offset = ph->p_offset;
			break;
		}
	}

	for (int i = 0; i < eh->e_phnum; i++) {
		Elf64_Phdr *ph = (Elf64_Phdr *)((char *)ls + (eh->e_phoff + eh->e_phentsize * i));
		if (ph->p_type == PT_DYNAMIC) {
			const Elf64_Dyn *dtag_table = (const Elf64_Dyn *)(ls + ph->p_offset);
			const char *strtab = NULL;

			for (int j = 0; 1; j++) {
				if (dtag_table[j].d_tag == DT_NULL) break;

				if (dtag_table[j].d_tag == DT_STRTAB) {
					const char *strtab_addr = (const char *)dtag_table[j].d_un.d_ptr;
					uint32_t strtab_offset = load_offset + (strtab_addr - load_addr);
					strtab = ls + strtab_offset;
				}
			}

			for (int j = 0; 1; j++) {
				if (dtag_table[j].d_tag == DT_NULL) break;

				if (dtag_table[j].d_tag == DT_NEEDED) 
					printf("shared lib: %s\n", &strtab[dtag_table[j].d_un.d_val]);
			}
			break;
		}
	}

	munmap((void *)ls, stat.st_size);

	return 0;
}