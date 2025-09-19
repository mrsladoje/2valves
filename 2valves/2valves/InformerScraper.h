#pragma once
#include "NewsScraper.h"
#include <cpr/cpr.h>

class InformerScraper : public NewsScraper {
public:
    /**
     * Fetches and extracts news article links from Informer.rs
     * @param url The URL to scrape (uses default if empty)
     * @return Vector of complete URLs to news articles
     */
    vector<string> fetchAndExtractNewsLinks() override;

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    vector<string> extractNewsLinks(const string& htmlContent) override;

private:
    const string DEFAULT_URL;
    const string BASE_URL = "https://informer.rs";

    /**
     * Fetches HTML content from a URL
     * @param url The URL to fetch
     * @return HTML content as string, or empty string on error
     */
    string fetchHtmlContent(const string& url);

    /**
     * Recursively searches for news-item-title elements and extracts links
     * @param node Current lexbor node to search
     * @param links Vector to store found links
     */
    void findNewsLinks(lxb_dom_node_t* node, vector<string>& links);
};