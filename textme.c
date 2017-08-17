#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define DIRNAME ".textme"

static char homedir[4096], data_dir[4096], phone_file[4096], provider_file[4096], msg_file[4096], phone[4096], provider[4096];

static void write_to_file(const char *filename, const char *data)
{
        FILE *file = fopen(filename, "w");
        fwrite(data, sizeof(char), strlen(data), file);
        fclose(file);
}

static void read_from_file(const char *filename, char *dst)
{
        FILE *file = fopen(filename, "r");
        fgets(dst, 1024, file);
        fclose(file);
}

static void copy_only_numbers(char *dst, const char *src)
{
	while(*src)
	{
		if (*src >= '0' && *src <= '9') 
		{
			*dst = *src;
			dst++;
		}
		src++;
	}
	*dst = '\0';
}

static void get_phone_number_from_user() {
	char buf[4096];
	printf("Enter your phone number: \n");
	fgets(buf, 4096, stdin);
	copy_only_numbers(phone, buf);
	write_to_file(phone_file, phone);
}

static void get_provider_from_user(){
	char buf[4096];
	printf("Enter your provider: \n");
	fgets(buf, 4096, stdin);
	strcpy(provider, buf);
	write_to_file(provider_file, buf);
}

static int load_config()
{
	if ((homedir = getenv("XDG_CONFIG_HOME")) == NULL)
	{
		if ((homedir = getenv("HOME")) == NULL)
		{
    			homedir = getpwuid(getuid())->pw_dir;
		}
	}
	sprintf(data_dir, "%s/%s", homedir, DIR_NAME);
	sprintf(phone_file,"%s/phone",data_dir);
	sprintf(provider_file, "%s/provider", data_dir);
	sprintf(msg_file,"%s/msg", data_dir);

	struct stat st = {0};

	if (stat(data_dir, &st) == -1)
	{
		int is_error;
		is_error = mkdir(data_dir, 0700);
		if (is_error == -1)
		{
			return CANNOT_OPEN_DIR_ERROR;
		}
		get_phone_number_from_user();
		get_provider_from_user();
		write_to_file(msg_file, "Build is finished!");
	}
	read_from_file(phone_file, phone);
	read_from_file(provider_file, provider);
}

static int main(int argc, char **argv)
{
	load_config();
	char cmd[4096];
	char *email = get_email();
	sprintf(cmd, "mail %s < %s", email, msg_file);
	system(cmd);
}
