#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#define PAGE_SIZE getpagesize()
#define MAPPAGE_SYSCALL_NUM 450
#define DEFAULT_SIZE 512
#define SUCCESS 0
#define FAIL -1
#define ERROR -1


int print_maps(void *address_start, void *address_end)
{
	size_t original_length = DEFAULT_SIZE;
	int result = 0;
	int should_continue = 0;
	char * buf_data = NULL;
	
	buf_data = (char *)malloc(original_length * sizeof(char));
	if (!buf_data)
	{
		printf("buf data allocation failed\n");

		return FAIL;	
	}
	
	do {
		result = syscall(MAPPAGE_SYSCALL_NUM ,address_start, address_end , buf_data, original_length);
		if (result < SUCCESS)
		{
			printf("syscall failed %d\n", result);
			
			return FAIL;
		}

		should_continue = 0;
		if (result == original_length)
		{
			free(buf_data);
			original_length *= 2;
			buf_data = (char *)malloc(original_length * sizeof(char));
			if (!buf_data)
			{
				printf("buf data allocation failed\n");

				return FAIL;
			}

			should_continue = 1;
		} 
	} while(should_continue);

	printf("%s\n", buf_data);
		
	free(buf_data);

	return SUCCESS;
}

int test1()
{
	const int PAGES_NUM = 10;
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long total_size = 0;

	printf("Test 1:\n" );
	printf("expecting: ..........\n");

	total_size = PAGE_SIZE * PAGES_NUM;
	
	ptr = mmap(KERNEL_CHOOSE_ADDR, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
    	printf("Mapping failed\n");
    	
    	return FAIL;
	}
	
	print_maps(ptr, ptr + total_size);
	
	if (munmap(ptr, total_size) == ERROR)
	{
		printf("munmap failed\n");
    	
    	return FAIL;	
	}
	
	return SUCCESS;
}

int test2()
{
	const int PAGES_NUM = 10;
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long total_size = 0;
	int i = 0;

	printf("Test 2:\n" );
	printf("expecting: 1111111111\n");

	total_size = PAGE_SIZE * PAGES_NUM;
	
	ptr = mmap(KERNEL_CHOOSE_ADDR, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
    	printf("Mapping failed\n");
    	
    	return FAIL;
	}

	for (i = 0; i < PAGES_NUM; ++i)
	{
		ptr[i * PAGE_SIZE] = 0;
	}
	
	print_maps(ptr, ptr + total_size);
	
	if (munmap(ptr, total_size) == ERROR)
	{
		printf("munmap failed\n");
    	
    	return FAIL;	
	}
	
	return SUCCESS;
}

int test3()
{
	const int PAGES_NUM = 10;
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long total_size = 0;
	int i = 0;

	printf("Test 3:\n" );
	printf("expecting: .1.1.1.1.1\n");

	total_size = PAGE_SIZE * PAGES_NUM;
	
	ptr = mmap(KERNEL_CHOOSE_ADDR, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
    	printf("Mapping failed\n");
    	
    	return FAIL;
	}

	for (i = 1; i < PAGES_NUM; i+=2)
	{
		ptr[i * PAGE_SIZE] = 0;
	}
	
	print_maps(ptr, ptr + total_size);
	
	if (munmap(ptr, total_size) == ERROR)
	{
		printf("munmap failed\n");
    	
    	return FAIL;	
	}
	
	return SUCCESS;
}

int test4()
{
	const int PAGES_NUM = 10;
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long total_size = 0;
	int i = 0;
	pid_t pid;

	printf("Test 4:\n" );
	printf("expecting: 22222.....\n");

	total_size = PAGE_SIZE * PAGES_NUM;
	
	ptr = mmap(KERNEL_CHOOSE_ADDR, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
    	printf("Mapping failed\n");
    	
    	return FAIL;
	}

	for (i = 0; i < PAGES_NUM / 2; ++i)
	{
		ptr[i * PAGE_SIZE] = 0;
	}

	pid = fork();
	if (pid == ERROR)
	{
		printf("fork failed\n");
    	
    	return FAIL;
	}

	if (pid == 0) 
	{
		print_maps(ptr, ptr + total_size);
		
		if (munmap(ptr, total_size) == ERROR)
		{
			printf("munmap failed\n");
    	
    		return FAIL;	
		}
		
		exit(0);
	} 
	else if (pid > 0) 
	{
		wait(NULL);
	}

	
	if (munmap(ptr, total_size) == ERROR)
	{
		printf("munmap failed\n");
    	
    	return FAIL;	
	}
	
	return SUCCESS;
}

int test5()
{	
	const int PAGES_NUM = 10;
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long total_size = 0;
	int i = 0;
	pid_t pid;

	printf("Test 5:\n" );
	printf("expecting: 1111..2222\n");

	total_size = PAGE_SIZE * PAGES_NUM;
	
	ptr = mmap(KERNEL_CHOOSE_ADDR, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
    	printf("Mapping failed\n");
    	
    	return FAIL;
	}

	for (i = 6; i < PAGES_NUM; ++i)
	{
		ptr[i * PAGE_SIZE] = 0;
	}

	pid = fork();
	if (pid == ERROR)
	{
		printf("fork failed\n");
    	
    	return FAIL;
	}

	if (pid == 0) 
	{
		for (i = 0; i < 4; ++i)
		{
			ptr[i * PAGE_SIZE] = 0;
		}

		print_maps(ptr, ptr + total_size);
		
		if (munmap(ptr, total_size) == ERROR)
		{
			printf("munmap failed\n");
    	
    		return FAIL;	
		}
		
		exit(0);
	} 
	else if (pid > 0) 
	{
		wait(NULL);
	}

	
	if (munmap(ptr, total_size) == ERROR)
	{
		printf("munmap failed\n");
    	
    	return FAIL;	
	}
	
	return SUCCESS;
}

int test6()
{
	const int PAGES_NUM = 2000;
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long total_size = 0;

	printf("Test 6:\n" );
	printf("expecting: exactly 2000 .\n");

	total_size = PAGE_SIZE * PAGES_NUM;
	
	ptr = mmap(KERNEL_CHOOSE_ADDR, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
    	printf("Mapping failed\n");
    	
    	return FAIL;
	}
	
	print_maps(ptr, ptr + total_size);
	
	if (munmap(ptr, total_size) == ERROR)
	{
		printf("munmap failed\n");
    	
    	return FAIL;	
	}
	
	return SUCCESS;
}

void test9()
{
	void *KERNEL_CHOOSE_ADDR = NULL;
	char *ptr = NULL;
	unsigned long page_numer = 20000000000;
	int i = 0;

	printf("Test 9:\n" );
	printf("expecting: triggers the Linux OOM (Out-Of-Memory) killer\n");
	
	while (1)
	{
		ptr = mmap(KERNEL_CHOOSE_ADDR, PAGE_SIZE * page_numer, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

		for (i = 0; i < page_numer; ++i)
		{
			ptr[i * PAGE_SIZE] = 0;
		}
	
		page_numer *= 2;
	}
}


int main(int argc, char **argv)
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test9();

	return SUCCESS;
}