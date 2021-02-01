###### HTTP Requester CLI Tool 
###### Author: Mark Chen (chenmark33@gmail.com)

BUILD & RUN INSTRUCTIONS: 

1.Navigate to the directory containing 'main.cpp' and 'makefile'  
2.Run 'make' (compiles for you) OR manually compile with either 'g++ main.cpp -std=c++11 -o main'
3.Start the program with './main --help' to check out the available flags and commands (--help flag's printout is given below) 

Usage:  
Compile using the included makefile with 'make' or with 'g++ main.cpp -o main' and run with:  
./main [FLAGS]

Flags:  
--url (Required) Takes in a URL string and makes an HTTP request to it  
--help (Optional) Prints this help message  
--profile (Optional) Takes in an integer and makes that number of requests to the URL passed into --url  
--verbose (Optional) When enabled, the IP address of the URL will be printed for each request 

EXAMPLES: 

./main --url google.com  
This makes a single HTTP request to Google 

./main --url cloudflare.com --profile 3  
This makes 3 HTTP requests to Cloudflare 

./main --url youtube.com --profile 2 --verbose 
This makes 2 HTTP requests with printouts of the full HTTP response, and 

Below are some sample responses from various links (omitting the HTML response for brevity).

mark@Marks-MBP-3 Systems Engineering Project % ./main --url google.com --profile 1
STATISTICS: 
Number of Requests: 1
Fastest Time: 22900 microseconds
Slowest Time: 22900 microseconds
Mean Time: 22900 microseconds 
Median Time: 22900 microseconds
Percentage of Successful Requests: 100%
Size of smallest response: 528 bytes
Size of largest response: 528 bytes
Error Codes (4xx and 5xx codes): 

mark@Marks-MBP-3 Systems Engineering Project % ./main --url facebook.com --profile 1
STATISTICS: 
Number of Requests: 1
Fastest Time: 29799 microseconds
Slowest Time: 29799 microseconds
Mean Time: 29799 microseconds 
Median Time: 29799 microseconds
Percentage of Successful Requests: 100%
Size of smallest response: 363 bytes
Size of largest response: 363 bytes
Error Codes (4xx and 5xx codes): 

mark@Marks-MBP-3 Systems Engineering Project % ./main --url walmart.com --profile 1
STATISTICS: 
Number of Requests: 1
Fastest Time: 56744 microseconds
Slowest Time: 56744 microseconds
Mean Time: 56744 microseconds 
Median Time: 56744 microseconds
Percentage of Successful Requests: 100%
Size of smallest response: 421 bytes
Size of largest response: 421 bytes
Error Codes (4xx and 5xx codes): 
