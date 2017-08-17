#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define DIRNAME ".textme"
#define PROVIDER_COUNT (23)

static char *homedir, data_dir[4096], phone_file[4096], provider_file[4096], msg_file[4096], username_file[4096], server_file[4096],
	    password_file[4096], from_address_file[4096], phone[4096], provider[4096], username[256], from_address[256],
	    port_file[4096], port[256], server[256],password[256];

static char int_to_provider[PROVIDER_COUNT][2][256] = 
{
	{"rogers", "pcs.rogers.com"},
	{"fido", "fido.ca"},
	{"telus", "msg.telus.com"},
	{"bell", "txt.bell.ca"},
	{"kudo","msg.koodomobile.com"},
	{"mts", "text.mtsmobility.com"},
        {"pc", "txt.bell.ca"},
	{"sasktel", "sms.sasktel.com"},
	{"solo", "txt.bell.ca"},
	{"virgin[CA]", "vmobile.ca"},
	{"at&t", "txt.att.net"},
	{"qwest", "qwestmp.com"},
	{"tmobile", "tmomail.net"},
	{"verizon", "vtext.com"},
	{"sprint", "messaging.sprintpcs.com"},
	{"virgin[US]", "vmobl.com"},
	{"nextel", "messaging.nextel.com"},
	{"alltel", "message.alltel.com"},
	{"metropcs", "mymetropcs.com"},
	{"powertel", "ptel.com"},
	{"suncom", "tms.suncom.com"},
	{"uscellular", "email.uscc.net"},
	{"cingular", "cingularme.com"}
};

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
	provider[strlen(provider) - 1] = '\0';
	write_to_file(provider_file, provider);
}

static void get_email_from_user()
{
	char buf[4096], *split;
	do
	{
	    printf("Enter your email: \n");
	    fgets(buf, 4096, stdin);
	    split = strchr(buf, '@');
	}
	while(NULL == split);

	//*split = '\0';
	strcpy(from_address, buf);
	from_address[strlen(from_address) - 1] = '\0';
	write_to_file(from_address_file, from_address);
}

static void get_server_from_user()
{
	printf("Enter your outgoing mail server:\n");
	char buf[4096];
	fgets(buf, 4096, stdin);
	strcpy(server, buf);
	server[strlen(server)-1] = '\0';
	write_to_file(server_file, server);
}

static void get_port_from_user()
{
	printf("Enter your outgoing mail server port:\n");
	char buf[256];
	fgets(buf, 256, stdin);
	copy_only_numbers(port, buf);
	write_to_file(port_file, port);
}

static void get_username_from_user()
{
	char buf[4096];
	printf("Enter your username: \n");
	fgets(buf, 4096, stdin);
	strcpy(username, buf);
	username[strlen(username) - 1] = '\0';
	write_to_file(username_file, username);
}

static void get_password_from_user()
{
        char buf[4096];
        printf("Enter your password: \n");
        fgets(buf, 4096, stdin);
        strcpy(password, buf);
        provider[strlen(password) - 1] = '\0';
        write_to_file(password_file, password);
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
	sprintf(data_dir, "%s/%s", homedir, DIRNAME);
	sprintf(phone_file,"%s/phone",data_dir);
	sprintf(provider_file, "%s/provider", data_dir);
	sprintf(username_file, "%s/username", data_dir);
	sprintf(server_file, "%s/server", data_dir);
	sprintf(password_file, "%s/password", data_dir);
	sprintf(from_address_file, "%s/from_address", data_dir);
	sprintf(port_file, "%s/port", data_dir);
	sprintf(msg_file,"%s/msg", data_dir);

	struct stat st = {0};

	if (stat(data_dir, &st) == -1)
	{
		int is_error;
		is_error = mkdir(data_dir, 0700);
		if (is_error == -1)
		{
			return 0;
		}
		get_phone_number_from_user();
		get_provider_from_user();
		get_email_from_user();
		get_server_from_user();
		get_port_from_user();
		get_username_from_user();
		get_password_from_user();
		write_to_file(msg_file, "Build is finished!");
	}
	read_from_file(phone_file, phone);
	read_from_file(provider_file, provider);
	read_from_file(username_file,username);
	read_from_file(server_file, server);
	read_from_file(password_file, password);
	read_from_file(from_address_file, from_address);
	read_from_file(port_file, port);
	return 1;
}

static int get_index_of_provider() 
{
	int index = 0;
	while (index < PROVIDER_COUNT &&
	       strcmp(int_to_provider[index][0],provider))
	{
		index++;
	}
	
	if (PROVIDER_COUNT == index)
	{
		return -1;
	}
	return index;
}

static char *get_email() {
	int index = get_index_of_provider();
	if (index < 0)
	{
		return NULL;
	}
	char *out = malloc(sizeof(char)*(12 + strlen(int_to_provider[index][1])));
	strcpy(out, phone);
	out[10] = '@';
	strcpy(out + 11, int_to_provider[index][1]);
	return out;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s command\n", argv[0]);
		return 1;
	}
	if (load_config()) 
	{
		char cmd[4096];

		char *email = get_email();
		if (NULL == email)
		{
			fprintf(stderr, "Provider %s does not have an email-to-text API\n", provider);
			return 1;
		}

		int result = system(argv[1]);
		sprintf(cmd, "/usr/local/bin/email -u '%s' -r %s -i '%s' -f '%s' -p %s -m plain --tls %s < '%s'",
			       	username, server, password, from_address, port, email, msg_file);
		//printf("%s\n", cmd);
		result = system(cmd);
		printf("%d\n", result);
		return 0;
	}
	else 
	{
		fprintf(stderr, "Problem creating the config directory\n");
		return 1;
	}
}
