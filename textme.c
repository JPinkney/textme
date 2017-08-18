#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <curl/curl.h>

#define DIRNAME ".textme"
#define PROVIDER_COUNT (23)

static char *homedir, data_dir[4096], phone_file[4096], provider_file[4096], msg_file[4096], username_file[4096], server_file[4096],
	    password_file[4096], from_address_file[4096], phone[4096], provider[4096], username[256], from_address[256],
	    port_file[4096], port[256], server[256],password[256], email_body[4096];

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
        password[strlen(password) - 1] = '\0';
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

static char *get_email_body(const char *email) {
	char *out = email_body;
	strcat(out, "This is an automatic notification for your build finishing. Do not reply to this message.");
	return out;
}

struct upload_status
{
	int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct upload_status *upload_ctx = (struct upload_status *)userp;
	const char *data;

	if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	const char *my_msg[] = {email_body, NULL};
	data = my_msg[upload_ctx->lines_read];

	if(data) {
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		upload_ctx->lines_read++;
		return len;
	}

	return 0;
}

static void send_email()
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;

	upload_ctx.lines_read = 0;
	curl = curl_easy_init();
	if(curl) {
	/* Set username and password */
	curl_easy_setopt(curl, CURLOPT_USERNAME, username);
	curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

	/* This is the URL for your mailserver. Note the use of port 587 here,
	* instead of the normal SMTP port (25). Port 587 is commonly used for
	* secure mail submission (see RFC4403), but you should use whatever
	* matches your server configuration. */
	char server_full[4096];
	server_full[0] = '\0';
	strcat(server_full, "smtp://");
	strcat(server_full, server);
	strcat(server_full, ":");
	strcat(server_full, port);

	curl_easy_setopt(curl, CURLOPT_URL, server_full);

	/* In this example, we'll start with a plain text connection, and upgrade
	* to Transport Layer Security (TLS) using the STARTTLS command. Be careful
	* of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
	* will continue anyway - see the security discussion in the libcurl
	* tutorial for more details. */
	curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

	/* If your server doesn't have a valid certificate, then you can disable
	* part of the Transport Layer Security protection by setting the
	* CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false).
	*   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	*   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	* That is, in general, a bad idea. It is still better than sending your
	* authentication details in plain text though.  Instead, you should get
	* the issuer certificate (or the host certificate if the certificate is
	* self-signed) and add it to the set of certificates that are known to
	* libcurl using CURLOPT_CAINFO and/or CURLOPT_CAPATH. See docs/SSLCERTS
	* for more information. */
    //curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");

    /* Note that this option isn't strictly required, omitting it will result
     * in libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise,
     * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
     * details.
     */
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_address);

    /* Add two recipients, in this particular case they correspond to the
     * To: and Cc: addressees in the header, but they could be any kind of
     * recipient. */

    char *email = get_email();
    get_email_body(email);

    recipients = curl_slist_append(recipients, email);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    /* We're using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* Since the traffic will be encrypted, it is very useful to turn on debug
     * information within libcurl to see what is happening during the transfer.
     */
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    /* Send the message */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    else {
	    printf("Message sent successfully!\n");
    }

    /* Free the list of recipients */
    curl_slist_free_all(recipients);

    /* Always cleanup */
    curl_easy_cleanup(curl);
  }
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
		int result = system(argv[1]);
		send_email();
		return 0;
	}
	else 
	{
		fprintf(stderr, "Problem creating the config directory\n");
		return 1;
	}
}
