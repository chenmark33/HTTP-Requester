#include <iostream> /* for io */
#include <cstdio> /* for printf */
#include <vector> /* for vectors */
#include <unordered_set> /* for storing error codes */
#include <getopt.h> /* CLI Tool Library */
#include <netdb.h> /* hostent structure */
#include <arpa/inet.h> /* Convert IPv4 and IPv6 addresses from binary to text form */
#include <chrono> /* for timing HTTP requests */
#include <numeric> /* for calculating mean using accumulator */

#define HTTPREQUEST(host, path) "GET /" + path + " HTTP/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n"
#define BUFFERSIZE 10000

using namespace std;

/**
 * Function Declarations
 */

/**
 * Prints the help guide
 * return void
 */
void printHelp();

/**
 * Splits a full URL into an HTTP-request-compatible host and path
 * return void
 */
void splitURL(char *charURL, string &host, string &path);

/**
 * Makes an HTTP request to url
 * return void
 */
void makeHTTPRequest(string url, bool isVerbose,
                     int &successes, int &smallestResponseByte, int &largestResponseByte,
                     unordered_set<string> &errorCodes, vector<int> &times, int &fastestTime, int &slowestTime);

/**
 * Calls makeHTTPRequest() a total of 'repeat' times and
 * calculates data such as request time(s), request number(s), and other information.
 * return void
 */
void repeatRequests(const string& url, int repeat, bool isVerbose);

/* Main */
int main(int argc, char *argv[]) {
    cout << "HTTP Requester CLI Tool" << endl;
    cout << "Author: Mark Chen (chenmark33@gmail.com)" << endl << endl;

    const struct option long_options[] = {
            {"url", required_argument, nullptr, 'u'},
            {"help", no_argument, nullptr, 'h'},
            {"profile", required_argument, nullptr, 'p'},
            {"verbose", no_argument, nullptr, 'v'},
            {0, 0, 0, 0}
    };

    string url;
    int option = 0, option_index = 0, repeat = 0;
    bool isVerbose = false;

    while (true) {
        option = getopt_long(argc, argv, "uhpv", long_options, &option_index);
        if (option == -1) {
            break;
        }
        switch (option) {
            default:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;
            case 'u':
                if (optarg) {
                    repeat = max(1, repeat);
                }
                url = optarg;
                break;
            case 'h':
                printHelp();
                break;
            case 'p':
                repeat = stoi(optarg);
                break;
            case 'v':
                isVerbose = true;
                break;
        }
    }
    if (repeat) {
        repeatRequests(url, repeat, isVerbose);
    }

    return 0;
}

void printHelp() {
    cout << "USAGE:" << endl;
    cout << "\t Compile with the included makefile or with 'g++ main.cpp -o main' and run with:" << endl;
    cout << "\t ./main [FLAGS]" << endl;

    cout << "\nFLAGS:" << endl;
    cout << "\t--url (Required) Takes in a URL string and makes an HTTP request to it" << endl;
    cout << "\t--help (Optional) Prints this help message" << endl;
    cout << "\t--profile (Optional) Takes in an integer and makes that number of requests to the URL passed into --url" << endl;
    cout << "\t--verbose (Optional) When enabled, the IP address of the URL will be printed for each request" << endl;
    cout << endl;
}

void splitURL(char *charURL, string &host, string &path) {
    string url = charURL;
    if (url.substr(0, 7) == "http://") {
        url.erase(0, 7);
    }
    if (url.substr(0, 8) == "https://") {
        url.erase(0, 8);
    }
    int delimiterIndex = url.find('/');
    if (delimiterIndex < url.size()) {
        host = url.substr(0, delimiterIndex);
        path = url.substr(delimiterIndex);
    }
    else {
        host = url;
        path = "";
    }
}

void makeHTTPRequest(string url, bool isVerbose, int &successes, int &smallestResponseByte, int &largestResponseByte,
                     unordered_set<string> &errorCodes, vector<int> &times, int &fastestTime, int &slowestTime) {
    int sockfd, // Socket that we wish to connect to
    addressError; // int indicating whether we successfully got the address info

    struct addrinfo hints, // Specifies criteria for selecting socket address structures returned
    *servinfo, // Pointer to a linked list of addrinfo structures that getaddrinfo() returns
    *ptr; // Pointer used to traverse servinfo

    char ip[INET6_ADDRSTRLEN]; // IP Address
    char buffer[BUFFERSIZE]; // Character buffer to hold HTTP response

    // Specify criteria for getaddrinfo() to use
    memset(&hints, 0, sizeof hints); // Zero out hints
    hints.ai_family = AF_UNSPEC; // Allow getaddrinfo() to find both IPv4 and IPv6 addresses
    hints.ai_socktype = SOCK_STREAM; // Preference for a connection-based TCP protocol

    string host, path;
    splitURL(&url[0], host, path); // Split URL into the host and path

    // Get address info using criteria in hints and place in servinfo
    addressError = getaddrinfo(&host[0], "80", &hints, &servinfo);
    if (addressError != 0) {
        cout << "Error getting address info for this port" << endl;
        return;
    }

    // Iterate through servinfo and connect to the first valid socket found
    for (ptr = servinfo; ptr != nullptr; ptr = ptr -> ai_next) {
        sockfd = socket(ptr -> ai_family, ptr -> ai_socktype, ptr -> ai_protocol);
        if (sockfd == -1) {
            addressError = errno;
            continue; // 'Continue' instead of 'break' since 'ai_family' might be different on the next iteration due to 'AF_UNSPEC'
        }
        int connectAttempt = connect(sockfd, ptr -> ai_addr, ptr -> ai_addrlen);
        if (connectAttempt == -1) {
            addressError = errno;
            close(sockfd);
            continue;
        }
        break;
    }

    // ptr did not find a valid address in servinfo
    if (ptr == nullptr) {
        cout << "Client failed to connect" << endl;
        return;
    }

    // Converts network address structure in ai_family into a character string
    inet_ntop(ptr -> ai_family, ptr -> ai_addr, ip, sizeof ip);

    if (isVerbose) {
        printf("Client connecting to %s\n", ip);
        printf("======\n");
    }

    // Free servinfo as we no longer need it
    freeaddrinfo(servinfo);

    // Construct HTTP message and time how long the request takes
    string msg = HTTPREQUEST(host, path);

    auto t1 = chrono::high_resolution_clock::now();
    send(sockfd, &msg[0], msg.size(), 0); // Sent HTTP request
    int bytesReceived = recv(sockfd, buffer, sizeof buffer, 0); // Receive HTTP response along with the number of bytes
    auto t2 = chrono::high_resolution_clock::now();

    int duration = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
    fastestTime = min(duration, fastestTime);
    slowestTime = max(duration, slowestTime);
    times.push_back(duration);
    cout << endl;

    if (bytesReceived == -1) {
        cout << "Error on receiving message" << endl;
        return;
    }

    // Print HTTP response message
    buffer[bytesReceived] = '\0';
    printf("%s", buffer);

    // Close socket
    close(sockfd);

    // Statistics
    successes++;
    smallestResponseByte = min(bytesReceived, smallestResponseByte);
    largestResponseByte = max(bytesReceived, largestResponseByte);
    string errorCode = string(buffer).substr(9, 3);
    if (errorCode[0] == '4' || errorCode[0] == '5') {
        errorCodes.insert(errorCode);
    }
}


void repeatRequests(const string& url, int repeat, bool isVerbose) {
    if (isVerbose) {
        cout << "VERBOSE FLAG: ON (URL IP ADDRESS WILL BE PRINTED)" << endl;
        cout << "Making " << repeat << " request(s) to " << url << endl << endl;
    }

    int numRequests = repeat,
            fastestTime = INT_MAX,
            slowestTime = 0,
            meanTime = 0,
            medianTime = 0,
            successes = 0,
            smallestResponseByte = 10000,
            largestResponseByte = 0;
    vector<int> times;
    unordered_set<string> errorCodes;

    for (int i = 0; i < repeat; ++i) {
        makeHTTPRequest(url, isVerbose, successes, smallestResponseByte, largestResponseByte, errorCodes, times, fastestTime, slowestTime);
    }

    sort(times.begin(), times.end());
    int n = times.size();
    if (n % 2 == 0) {
        medianTime = (times[n / 2 - 1] + times[n / 2]) / 2;
    }
    else {
        medianTime = times[n / 2];
    }
    meanTime = accumulate(times.begin(), times.end(), 0) / n;

    cout << endl << "STATISTICS: " << endl;
    cout << "Number of Requests: " << numRequests << endl;
    cout << "Fastest Time: " << fastestTime << " microseconds" << endl;
    cout << "Slowest Time: " << slowestTime << " microseconds" << endl;
    cout << "Mean Time: " << meanTime << " microseconds " << endl;
    cout << "Median Time: " << medianTime << " microseconds" << endl;
    cout << "Percentage of Successful Requests: " << (successes / numRequests) * 100 << "%" << endl;
    cout << "Size of smallest response: " << smallestResponseByte << " bytes" << endl;
    cout << "Size of largest response: " << largestResponseByte << " bytes" << endl;
    cout << "Error Codes (4xx and 5xx codes): " << endl;
    for (auto & code : errorCodes) {
        cout << code << endl;
    }
    cout << endl;
}