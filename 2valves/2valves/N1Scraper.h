#pragma once

#include "NewsScraper.h"
#include <cpr/cpr.h>

class N1Scraper : public NewsScraper {
public:
    /**
     * Fetches and extracts news article links from N1info.rs
     * @return Vector of complete URLs to news articles
     */
    vector<string> fetchAndExtractNewsLinks() override;

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    vector<string> extractNewsLinks(const string& htmlContent) override;


    string fetchAndExtractArticleContent(const string& url) override;

private:
    const string BASE_URL = "https://n1info.rs";

    /**
     * Fetches HTML content from a URL
     * @param url The URL to fetch
     * @return HTML content as string, or empty string on error
     */
    string fetchHtmlContent(const string& url);

    /**
     * Recursively searches for article elements and extracts links
     * @param node Current lexbor node to search
     * @param links Vector to store found links
     */
    void findNewsLinks(lxb_dom_node_t* node, vector<string>& links);
};