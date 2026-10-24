#include "http_client.h"
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::pair<int, std::string> HttpClient::get(const std::string& url, const std::string& authToken, long timeoutSec) {
    CURL* curl = curl_easy_init();
    if (!curl) return {-1, ""};
    std::string response;
    struct curl_slist* headers = nullptr;
    if (!authToken.empty()) headers = curl_slist_append(headers, ("X-Auth-Token: " + authToken).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSec);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    if (headers) curl_slist_free_all(headers);

    return (res == CURLE_OK) ? std::make_pair((int)http_code, response) : std::make_pair(-1, std::string(""));
}

std::pair<int, std::string> HttpClient::post(const std::string& url, const std::string& body, const std::string& authToken, long timeoutSec) {
    CURL* curl = curl_easy_init();
    if (!curl) return {-1, ""};
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!authToken.empty()) headers = curl_slist_append(headers, ("X-Auth-Token: " + authToken).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSec);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    if (headers) curl_slist_free_all(headers);

    return (res == CURLE_OK) ? std::make_pair((int)http_code, response) : std::make_pair(-1, std::string(""));
}