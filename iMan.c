
#include "initialise.h"

#define BUFFER_SIZE 4096

// Function to fetch and print the man page for a given command in a specific section
void fetch_man_page_for_section(const char *command, const char *section, int *found)
{
    int sockfd;
    struct addrinfo hints, *res;
    char request[BUFFER_SIZE], buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    char *body_start = NULL;
    int header_end = 0;
    int status_code = 0;

    // Prepare the GET request
    snprintf(request, sizeof(request),
             "GET /%s/%s HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n",
             section, command);

    // Set up hints for the getaddrinfo call
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Get address information for the host (man.he.net)
    int addr_status = getaddrinfo("man.he.net", "80", &hints, &res);
    if (addr_status != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(addr_status));
        exit(EXIT_FAILURE);
    }

    // Create the socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("connect failed");
        close(sockfd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Free the address information
    freeaddrinfo(res);

    // Send the GET request to the server
    if (send(sockfd, request, strlen(request), 0) < 0)
    {
        perror("send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive the response and process the body
    // printf("Request sent for section %s. Waiting for response...\n", section);

    int skip_mode = 0; // Flag to determine if we are inside < and >

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0'; // Null-terminate the buffer

        if (!header_end)
        {
            // Check for the end of headers
            body_start = strstr(buffer, "\n\n");
            if (body_start)
            {
                header_end = 1;
                body_start += 2; // Skip past "\n\n"

                // Check for HTTP status code in the response
                char *status_line = strstr(buffer, "HTTP/1.1 ");
                if (status_line)
                {
                    sscanf(status_line, "HTTP/1.1 %d", &status_code);
                    if (status_code == 200)
                    {
                        // Initialize printing state
                        if (strchr(body_start, '<'))
                        {
                            skip_mode = 1;
                        }
                        printf("%s", body_start);
                        *found = 1;
                    }

                    else if (status_code == 404)
                    {
                        printf("No matches for \"%s\"\n", command);
                        printf("Search Again\n");
                        *found = 0;
                    }
                    else
                    {
                        printf("Received HTTP status code %d for section %s.\n", status_code, section);
                        *found = 0;
                    }
                }
            }
        }
        else
        {
            // Process the body with skipping logic
            for (ssize_t i = 0; i < bytes_received; i++)
            {
                if (buffer[i] == '<')
                {
                    skip_mode = 1;
                }
                else if (buffer[i] == '>')
                {
                    skip_mode = 0;
                }
                else if (!skip_mode)
                {
                    putchar(buffer[i]);
                }
            }
            *found = 1;
        }
    }

    // ...
    if (bytes_received < 0)
    {
        perror("recv failed");
    }

    // Close the socket
    close(sockfd);
}

// Function to fetch and print the man page for a given command in all sections
void fetch_man_page(const char *command)
{
    char section[10];
    int found = 0;

    // Iterate through sections 1 to 9
    for (int i = 1; i <= 9; i++)
    {
        snprintf(section, sizeof(section), "man%d", i);
        fetch_man_page_for_section(command, section, &found);
        if (found)
            break;
    }

    if (!found)
    {

        printf("%s\n\n",command);
        printf("%s\n\n",command);
        printf("No matches for \"%s\"\n", command);
        printf("Search Again\n\n");
        printf("Man Pages Copyright Respective Owners.\n");
        printf("Site Copyright (C) 1994 - 2024 Hurricane Electric.\n");
        printf("All Rights Reserved.\n\n");
        printf("ERROR: No matches for \"%s\" command\n", command);
    }
}
