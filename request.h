typedef struct __attribute__((__packed__)) _msg{
	unsigned char sender[10];
	unsigned char subject_size[8];
	unsigned char text_size[64];
	unsigned char subj_and_text[];
}MSG;

typedef struct __attribute__((__packed__)) _auth{
	unsigned char username[10];
	unsigned char password[10]; //ashed
}AUTH;

typedef struct __attribute__((__packed__)) _req{
	unsigned char req_type[1];
	unsigned char data[];
}REQ;

//type of requests:
//0 := request compatibility (check if the server is compatible with the client)
//1 := request options
//n > 1 means "select option n-1"
//2 := request all messages (when already sent this request, the request type 2 is used to request another batch of messages; that's because the server will send only n messages), in data there will be the number n
//3 := send a message, in data there will be the message struct;
//4 := authenticate, in date there will be an auth struct
//5 := Remove message, in data there will be the number of the message (10 bit)

typedef struct __attribute__((__packed__)) _options{
	char option_number[1];
	char option_name[20];
	char description_size[5];
	char description[];
}options;

//options:
//1 := Read all messages
//2 := Send a message
//3 := Authenticate
//4 := Remove a message

typedef struct __attribute__((__packed__)) _resp{
	char resp_type[1];
	char data[];
}RESP;

//type of responses:
//0 := compatibility response, data should have string COMPATIBLE! (11 byte) + Server Version (3 byte) 
//1 := sending options, data has a 2 byte string with the number of options, then there is an array of options structs;
//2 := message sent, the first n (n comes from the request, it could be even less) msg structs are sent, the last byte in data is 0 if the user has read all messages or 1 if there are more
//3 := message received, in data there is the number of the message (10 bit)
//4 := authentication successful (SUCCESSFUL in data) or failed (FAIL in data)
//5 := if in data there is the same number of the deleted message the request is successful, if there is 0 the message was not found, if the user is not authenticated the string "NO AUTH" will be present