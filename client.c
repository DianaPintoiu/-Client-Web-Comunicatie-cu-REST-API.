#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <ctype.h>

//I want to make a struct that the fields of a JSON object
typedef struct {
    int size;
    JSON_Value *root_value;
}JsonVector;


int main(int argc, char *argv[])
{
    char *message;
    //din laborator 
    char *response = calloc(BUFLEN, sizeof(char));
    //asta este ce fel de request o sa facem(dat de la tastatura)
    char type[100];
    int sockfd;
    //cookieurile mele sunt un vector de stringuri
    char **cookies;
    cookies = calloc(10000, sizeof(char*) );
    for(int i = 0; i < 10000; i++)
    {
        cookies[i] = calloc(10000, sizeof(char));
    }
    //body_data-ul este un vector de stringuri
    //si in el primesc ce vreau sa trimit la server
    //adica username si parola sau ce trebuie pentru carti(vezi add book)
    char **body_data;
    body_data = calloc(100, sizeof(char*) );
    for(int i = 0; i < 100; i++)
    {
        body_data[i] = calloc(100, sizeof(char));
    }

    int response_code;
    char *response_aux = calloc(100, sizeof(char));
    char token[1000000];
    //acesti 4 pointeri sunt pentru strtok_r
    //care are nevoie sa tina minte ce urmeaza dupa delimitator pentru a putea face
    //2 strtok_r la rand
    char *addr1;
    char *addr2;
    char *addr3;
    char *addr4;
    while(1)
    {
        //deschidem conexiunea
        sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
        //vedem ce vrem sa facem
         scanf("%s", type);
         if(strcmp(type,"register") == 0)
         {

                printf("username=");
                scanf("%s", body_data[0]);
                printf("password=");
                scanf("%s", body_data[1]);
                //am dat un post_request cu body_data[0] si body_data[1] ca sa imi fac cont
                message = compute_post_request("34.254.242.81","/api/v1/tema/auth/register","application/json",body_data,2,NULL,0,type,NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                sscanf(response, "%s %d",response_aux,&response_code);
                if(response_code == 200 || response_code == 201)
                {
                    printf("%d - OK - Utilizator înregistrat cu succes!”.\n",response_code);
                }
                else
                {   if(response_code == 500)
                    {
                        printf("500 - Internal Server Error\n");
                    }
                    else
                    printf("%d - Invalid username or password\n",response_code);
                }
         } else
         if(strcmp(type,"login") == 0) 
         {
             printf("username=");
             scanf("%s", body_data[0]);
             printf("password=");
             scanf("%s", body_data[1]);
             //am dat un post request cu body_data[0] si body_data[1] ca sa ma autentific
             message = compute_post_request("34.254.242.81","/api/v1/tema/auth/login","application/json",body_data,2,NULL,0,type,NULL);
             send_to_server(sockfd, message);
             response = receive_from_server(sockfd);
             sscanf(response, "%s %d",response_aux,&response_code);
             if(response_code == 200 || response_code == 201)
             {
                 printf("%d - OK - Utilizator autentificat cu succes!\n",response_code);
                 //se intoarce sesiunea de cookies
                char *p = strtok(response, " ");
                int i = 0;
                while(p != NULL)
                {
                    if(i == 12)
                    {
                        strncpy(cookies[0],p,strlen(p) - 1);
                    }
                    p = strtok(NULL, " ");
                    i++;
                }
             }
             else
             {
                if(response_code == 500)
                    {
                        printf("500 - Internal Server Error\n");
                    }
                else
                 printf("%d - Invalid username or password. Try again.\n",response_code);
             }

         }else
         if(strcmp(type,"enter_library") == 0)
         {
                //am dat un get request cu cookies[0] ca sa intru in biblioteca
                //adica pot sa intru in biblioteca doar daca sunt autentificat
             message = compute_get_request("34.254.242.81","/api/v1/tema/library/access",NULL,cookies,1,NULL);
             send_to_server(sockfd, message);
             response = receive_from_server(sockfd);
             sscanf(response, "%s %d",response_aux,&response_code);
             if(response_code == 200 ||  response_code == 201)
             {
                printf("%d - OK - Acces allowed. You entered the library!\n",response_code);
                //urmeaza salvarea tokenului
                    char *p = strtok_r(response, "\n",&addr1);
                    int i = 0;
                    while(p != NULL)
                    {
                        if(i == 15)
                        {
                            char *aux = strtok_r(p, "\"",&addr2);
                            int j = 0;
                            while(aux!=NULL)
                            {
                                if(j == 2)
                                {
                                    aux = strtok_r(NULL, "\"",&addr2);
                                    strncpy(token,aux,strlen(aux));
                                }
                                aux = strtok_r(NULL, "\"",&addr2);
                                j++;
                            }
                            strncpy(token,p,strlen(p) - 1);
                        }
                        p = strtok_r(NULL, "\n",&addr1);
                        i++;
                    }
             } else
             {
                    printf("%d - Acces denied. Try again.\n",response_code);
                    
             }
         }
         else
         if(strcmp(type,"get_books") == 0)
         {
             message = compute_get_request("34.254.242.81","/api/v1/tema/library/books",NULL,cookies,1,token);
             send_to_server(sockfd, message);
             response = receive_from_server(sockfd);
             sscanf(response, "%s %d",response_aux,&response_code);
            if(response_code == 200 || response_code == 201)
            {
                
                //intoarce o lista de obiecte de tip json
                JsonVector *vector;
                vector = calloc(99999999,sizeof(JsonVector));
                int i = 0;
                char *p = strtok_r(response, "[",&addr3);
                //daca addr3 == ] inseamna ca nu exista carti in biblioteca
                if(strcmp(addr3,"]") == 0)
                {
                    printf("Nu exista carti in biblioteca!\n");
                    continue;
                }
                printf("%s\n",addr3);
                while(p != NULL)
                {
                    char *aux = strtok_r(addr3, "{",&addr4);
                    while(aux != NULL)
                    {   
                        JSON_Object *obj;
                        vector[i].root_value = json_value_init_object();
                        obj = json_value_get_object(vector[i].root_value);
                        //acum iau campul id
                        char id[100];
                        int index = 0;
                        while(aux[index + 5] != ',')
                        {
                            
                            id[index] = aux[index + 5];
                            index++;
                        }
                        //il transform in int
                        json_object_set_number(obj, "id", atoi(id));
                        //acum iau campul title
                        char title[1000];
                        //iau aux + index(cat e id) + 5 (pt {"id": ) + 9 (pt ca e ,"title": ) + 1 ([pentru "")
                        //iar pentru cat luam luam lungimea lui aux minus tot ce am sters pana acum si mai stegem
                        //ultimele 4 caractere 3 pentru "}, 
                        strncpy(title,aux + index + 5 + 9 + 1,strlen(aux) - 9 -5 - 1 - index - 3 );
                        //il pun in json
                        json_object_set_string(obj, "title", title);
                        memset(title,0,1000);
                        aux = strtok_r(NULL, "{",&addr4); 
                        i++;
                    }
                    p = strtok_r(NULL, "[",&addr3);
                }
                printf("%d - OK - Books retrieved successfully!\n",response_code);
                //afisez vectorul de jsonuri
                printf("[");
                for(int j = 0; j < i; j++)
                {
                    printf("%s\n",json_serialize_to_string_pretty(vector[j].root_value));
                }
                printf("]\n");
            }
            else
            {
                printf("%d - Cannot get the books. You have no authorization\n",response_code);
            }
         }
         else
         if(strcmp(type,"get_book") == 0)
         {
             printf("id=");
             scanf("%s", body_data[0]);
             //acum se face evaluarea id-ului
             int isNumber = 1;
             for(int i = 0; i < strlen(body_data[0]); i++)
             {
                //daca nu e cifra inseamna ca nu e numar
                if(body_data[0][i] < '0' || body_data[0][i] > '9')
                {
                    isNumber = 0;
                    break;
                }
             }
             //daca nu e numar e invalid id-ul
             if(isNumber == 0)
            {
                printf("Invalid id!\n");
                continue;
            }
            char *url = calloc(1000,sizeof(char));
            strcpy(url,"/api/v1/tema/library/books/");
            strcat(url,body_data[0]);
            //fac un get request cu id-ul ca sa imi ia cartea cu acel id
             message = compute_get_request("34.254.242.81",url,NULL,cookies,1,token);
             send_to_server(sockfd, message);
             response = receive_from_server(sockfd);
             sscanf(response, "%s %d",response_aux,&response_code);
            int i = 0;
             if(response_code == 200 || response_code == 201)
             {
                //prelucrare response pentru a afisa cartea cu id-ul body_data[0]
                printf("%d - OK - Book with id: %s retrieved successfully!\n",response_code,body_data[0]);
                char *p = strtok(response, "\n");
                while(p !=NULL)
                {
                    //aici se gaseste responsul, adica cartea
                    if(i == 15)
                    {
                        
                        char *serialized_string = NULL;
                        //baga din string in json
                        JSON_Value *root_value = json_parse_string(p);
                        //si apoi din nou in string dar pretty
                        serialized_string = json_serialize_to_string_pretty(root_value);
                        printf("%s\n",serialized_string);
                    }
                    p = strtok(NULL, "\n");
                    i++;
                }
             }
             else
             {
                if(response_code == 404)
                printf("%d - Book not found!\n",response_code);
                else
                printf("%d - Cannot get the book. You have no authorization\n",response_code);
             }
         }
         else
         if(strcmp(type,"add_book") == 0)
         {
                //trebuie sa dau flush la stdin
                int c = getchar();
                while (c != '\n' && c != EOF) {
                    c = getchar();
                }
                //se citesc datele cartii
                printf("title=");
                fgets(body_data[0],LINELEN,stdin);
                //asta e o verificare daca nu e nimic in title
                //atunci nu e bine si se va incerca din nou
                if(strcmp( body_data[0] , "\n") == 0)
                {
                    printf("Invalid title!\n");
                    continue;
                }
                 //acum o sa dam remove la newspace
                body_data[0][strcspn(body_data[0], "\n")] = '\0';
                printf("author=");
                fgets(body_data[1],LINELEN,stdin);
                if(strcmp( body_data[1] , "\n") == 0)
                {
                    printf("Invalid author!\n");
                    continue;
                }
                body_data[1][strcspn(body_data[1], "\n")] = '\0';
                printf("genre=");
                fgets(body_data[2],LINELEN,stdin);
                if(strcmp(body_data[2], "\n") == 0)
                {
                    printf("Invalid genre!\n");
                    continue;
                }

                body_data[2][strcspn(body_data[2], "\n")] = '\0';

                printf("page_count=");
                fgets(body_data[3],LINELEN,stdin);
                if(strcmp(body_data[3], "\n") == 0)
                {
                    printf("Invalid page_count!\n");
                    continue;
                }

                body_data[3][strcspn(body_data[3], "\n")] = '\0';

                //acum se face evaluarea page_count-ului
                int isNumber = 1;
                for(int i = 0; i < strlen(body_data[3]); i++)
                {
                    //daca nu e cifra inseamna ca nu e numar
                    if(body_data[3][i] < '0' || body_data[3][i] > '9')
                    {
                        isNumber = 0;
                        break;
                    }
                }
                //daca nu e numar e invalid id-ul
                if(isNumber == 0)
                {
                    printf("Invalid page_count! Should be a number\n");
                    continue;
                }

                printf("publisher=");
                fgets(body_data[4],LINELEN,stdin);
                if(strcmp(body_data[4], "\n") == 0)
                {
                    printf("Invalid publisher!\n");
                    continue;
                }
                body_data[4][strcspn(body_data[4], "\n")] = '\0';
                //se face post request cu datele cartii
                message = compute_post_request("34.254.242.81","/api/v1/tema/library/books","application/json",body_data,5,cookies,1,type,token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                sscanf(response, "%s %d",response_aux,&response_code);
                if(response_code == 200 || response_code == 201)
                {
                    printf("%d - OK - Book added successfully!\n",response_code);
                }
                else
                {
                    printf("%d - Cannot add the book. You have no authorization\n",response_code);
                }
         }
         else 
         if(strcmp(type,"delete_book") == 0)
         {
             printf("id=");
             scanf("%s", body_data[0]);
             //acum se face evaluarea id-ului
             int isNumber = 1;
             for(int i = 0; i < strlen(body_data[0]); i++)
             {
                //daca nu e cifra inseamna ca nu e numar
                if(body_data[0][i] < '0' || body_data[0][i] > '9')
                {
                    isNumber = 0;
                    break;
                }
             }
             //daca nu e numar e invalid id-ul
             if(isNumber == 0)
            {
                printf("Invalid id!\n");
                continue;
            }
            char *url = calloc(1000,sizeof(char));
            strcpy(url,"/api/v1/tema/library/books/");
            strcat(url,body_data[0]);
            //se face delete request cu id-ul cartii
            message = compute_delete_request("34.254.242.81",url,NULL,cookies,1,token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            sscanf(response, "%s %d",response_aux,&response_code);
            if(response_code ==200 || response_code == 201)
            {
                printf("%d - OK - Book deleted successfully!\n",response_code);
            }
            else
            {
                if(response_code == 404)
                printf("%d - Book not found!\n",response_code);
                else
                printf("%d - Cannot delete the book. You have no authorization\n",response_code);
            }
         }
         else
         if(strcmp(type,"logout") == 0)
         {
             //se face get request pentru logout
             message = compute_get_request("34.254.242.81","/api/v1/tema/auth/logout",NULL,cookies,1,token);
             send_to_server(sockfd, message);
             response = receive_from_server(sockfd);
             sscanf(response, "%s %d",response_aux,&response_code);
             if(response_code == 200 || response_code == 201)
             {
                printf("%d - OK - You have been logged out!\n",response_code);
                //trebuie sa sterg cookie-ul
                cookies = calloc(10000,sizeof(char*));
                for(int i = 0; i < 10000; i++)
                {
                    cookies[i] = calloc(1000,sizeof(char));
                }

                //trebuie sa sterg token-ul
                memset(token,0,1000);
             }
             else
             {
                printf("%d - Cannot logout. Something happend. Try again later.\n",response_code);
             }
         }
         else
         if(strcmp(type,"exit") == 0)
         {
            //daca e exit dam break iesim din tot
             break;
         }
         else
         {
            //orice alta vcomanda e invalida deci se va incerca din nou
             printf("Invalid command\n");
         }
    close_connection(sockfd);
    }

    // free the allocated data at the end!
    for(int i = 0; i < 100; i++)
    {
        free(body_data[i]);
    }
    free(body_data);
    for(int i = 0; i < 10000; i++)
    {
        free(cookies[i]);
    }
    free(cookies);
    return 0;
}
