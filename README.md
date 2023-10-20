# -Client-Web-Comunicatie-cu-REST-API.
## Overview

This repository contains the code for a Client-Web Comunication with REST-API. The project includes the following files:

- **Request.c:** Contains functions `compute_delete_request`, `compute_post_request`, and `compute_get_request`. These functions handle sending the necessary information to the server and composing the message to be sent.

    - `compute_get_request`: 
        - The first line of the message contains "GET" with the URL and query parameters.
        - The second line specifies the host, which, in our case, is "34.254.242.81."
        - Next, it includes cookies if available, and it frees all allocated pointers.

    - `compute_post_request`:
        - The first line of the message contains "POST" with the URL and query parameters.
        - The second line specifies the host.
        - It includes authorization, if available (the token).
        - There are three cases:
            1) If "login" is read from the keyboard:
                - The parson.c/parson.h library is used to create a JSON with username and password.
                - A JSON_Value is created to set the received parameters as strings in body_data. It serializes them as a string with proper formatting.
                - The JSON is freed.
            2) If "register" is read from the keyboard:
                - The parson.c/parson.h library is used to create a JSON with the token.
                - Similar to the login case, a JSON_Value is used to create and set the received parameters as strings in body_data.
                - The JSON is freed.
            3) If "add_book" is read from the keyboard:
                - The parson.c/parson.h library is used to create a JSON with the token.
                - Similar to the other cases, a JSON_Value is used to create and set the received parameters as strings in body_data (now multiple parameters). It serializes them as a string with proper formatting.
                - The JSON is freed.
        - It includes cookies if available and the content type ("application/json") and content length (the length of the JSON). Finally, it frees all allocated pointers.

    - `compute_delete_request`:
        - The first line of the message contains "DELETE" with the URL and query parameters.
        - The second line specifies the host.
        - It includes authorization, if available (the token).
        - It includes cookies if available. It is similar to the GET request, but without freeing any memory.

- **parson.c/parson.h Library:**
    - A library used to create JSON objects.
    - Utilized in compute_post_request, compute_get_request, compute_delete_request, and get_books to create JSON objects.

- **Client.c:** The main file of the project that establishes a connection with the server and handles user interactions. It reads user input and performs various actions based on the input:

    - **Register:** Reads and sends username and password for user registration.
    - **Login:** Reads and sends username and password for user login. Saves the received cookie for use in the enter_library request.
    - **Enter_library:** Sends a request to access the library. Saves the received token for use in subsequent requests. Provides feedback if the login was unsuccessful.
    - **Get_books:** Sends a request to get the list of books and parses the response to display book IDs and titles.
    - **Get_book:** Reads and sends the book ID and displays detailed information about the book.
    - **Add_book:** Reads and sends book information to add a new book.
    - **Delete_book:** Reads and sends the book ID to delete a book.
    - **Logout:** Logs out and clears the cookies and token.
    - **Exit:** Exits the program.
    - Invalid commands display an appropriate message.
    - The program closes the server connection and frees dynamically allocated variables.

## Testing

During testing, the username "Alina_Diana" and password "Pintoiu" were used. Multiple books were added for extensive testing.
