#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

/**
 * Abstract interface for news website scrapers
 */
class NewsScraper {
public:
    virtual ~NewsScraper() = default;

    /**
     * Fetches and extracts news article links from the website
     * @param url The URL to scrape (uses default if empty)
     * @return Vector of complete URLs to news articles
     */
    virtual vector<string> fetchAndExtractNewsLinks(const string& url = "") = 0;

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    virtual vector<string> extractNewsLinks(const string& htmlContent) = 0;
};