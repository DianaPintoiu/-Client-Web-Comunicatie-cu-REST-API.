#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count,char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *hostline = calloc(LINELEN, sizeof(char));
    char *authline = calloc(LINELEN, sizeof(char));
    char *cookiesline = calloc(LINELEN, sizeof(char));
    //write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    
    //add the host
    sprintf(hostline, "Host: %s", host);
    compute_message(message, hostline);
    //add the authorization
    if (auth != NULL) {
        strcat(authline, "Authorization: Bearer ");
        strcat(authline, auth);
        compute_message(message, authline);
    }
    //add cookies, according to the protocol format
    if (cookies != NULL) {
       strcpy(cookiesline, "Cookie: ");
       for(int i = 0; i < cookies_count; i++) {
           strcat(cookiesline, cookies[i]);
       }
         compute_message(message, cookiesline);
    }
    // add final new line
    compute_message(message, "");
    free(line);
    free(hostline);
    free(cookiesline);
    free(authline);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *type,char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
    char *hostline = calloc(LINELEN, sizeof(char));
    char *authline = calloc(LINELEN, sizeof(char));
    char *cookiesline = calloc(LINELEN, sizeof(char));
    int body_data_len = 0;

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    //add the host
    sprintf(hostline, "Host: %s", host);
    compute_message(message, hostline);

   //add the authorization
    if (auth != NULL) {
        strcpy(authline, "Authorization: Bearer ");
        strcat(authline, auth);
        compute_message(message, authline);
    }
   //am body_data care pt login si register are doar 2 campuri
   //username si password
   //si le fac de forma unui JsonObject
   if(strcmp(type,"login") == 0) {
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *serialized_string = NULL;
        json_object_set_string(root_object, "username", body_data[0]);
        json_object_set_string(root_object, "password", body_data[1]);
         //si acum serializez obiectul sa fie de tip string
        serialized_string = json_serialize_to_string_pretty(root_value);
        //il bag in body_data_buffer ca sa il trimit mai departe
        strcpy(body_data_buffer, serialized_string);
        body_data_len = strlen(body_data_buffer);  
        //puts(serialized_string);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
   }
   else 
   //pentru register fac aceeasi chestie
   if(strcmp(type,"register") == 0) {
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *serialized_string = NULL;
        //char serialized_string[] = "{\"username\":\"test1\",\"password\":\"test1\"}";
        json_object_set_string(root_object, "username", body_data[0]);
        json_object_set_string(root_object, "password", body_data[1]);
         //si acum serializez obiectul sa fie de tip string
        serialized_string = json_serialize_to_string_pretty(root_value);
        //il bag in body_data_buffer ca sa il trimit mai departe
        strcpy(body_data_buffer, serialized_string);
        body_data_len = strlen(body_data_buffer);  
        //puts(serialized_string);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
   }
   else
   if(strcmp(type,"add_book") == 0) {
        //aici se va serializa un obiect de tip book intr-un json
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *serialized_string = NULL;
        json_object_set_string(root_object, "title", body_data[0]);
        json_object_set_string(root_object, "author", body_data[1]);
        json_object_set_string(root_object, "genre", body_data[2]);
        json_object_set_number(root_object, "page_count", atoi(body_data[3]));
        json_object_set_string(root_object, "publisher", body_data[4]);

         //si acum serializez obiectul sa fie de tip string
        serialized_string = json_serialize_to_string_pretty(root_value);
        //aici la del ca la register si login doar ca am mai multe informatii
        strcpy(body_data_buffer, serialized_string); 
        //voi avea nevoie de lungimea lui pentru content length
        body_data_len = strlen(body_data_buffer);  
        //dam free la tot ce trebuie 
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
   }
    
    
    // add cookies
     if (cookies != NULL) {
       strcpy(cookiesline, "Cookie: ");
       for(int i = 0; i < cookies_count; i++) {
           strcat(cookiesline, cookies[i]);
       }
         compute_message(message, cookiesline);
    }
    /* add necessary headers (Content-Type and Content-Length are mandatory)
    in order to write Content-Length you must first compute the message size
    */
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s\r\nContent-Length: %d", content_type, body_data_len);
    compute_message(message, line);
    // add new line at end of header
    // Step 6: add the actual payload data
    compute_message(message, "");
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    free(hostline);
    free(cookiesline);
    free(authline);
    return message;
}



//este aproape la fel ca la get doar ca am Delete in loc de Get
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count,char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *hostline = calloc(LINELEN, sizeof(char));
    char *authline = calloc(LINELEN, sizeof(char));
    char *cookiesline = calloc(LINELEN, sizeof(char));
    //write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);
    
    // add the host
    sprintf(hostline, "Host: %s", host);
    compute_message(message, hostline);
    //add the authorization
    if (auth != NULL) {
        strcat(authline, "Authorization: Bearer ");
        strcat(authline, auth);
        compute_message(message, authline);
    }
    //add cookies, according to the protocol format
    if (cookies != NULL) {
       strcpy(cookiesline, "Cookie: ");
       for(int i = 0; i < cookies_count; i++) {
           strcat(cookiesline, cookies[i]);
       }
         compute_message(message, cookiesline);
    }
    // add final new line
    compute_message(message, "");

    return message;
}
