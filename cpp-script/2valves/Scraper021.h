#pragma once
#include "NewsScraper.h"
#include <cpr/cpr.h>

class Scraper021 : public NewsScraper {
public:
    /**
     * Fetches and extracts news article links from 021.rs
     * @return Vector of complete URLs to news articles
     */
    vector<string> fetchAndExtractNewsLinks() override;

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    vector<string> extractNewsLinks(const string& htmlContent) override;

    /**
     * Fetches and extracts the full content of a news article from 021.rs
     * @param url The URL of the article to scrape
     * @return The article content as a formatted string, or empty string on error
     */
    string fetchAndExtractArticleContent(const string& url) override;

    /**
     * Extracts the article published date from the parsed HTML
     * @param root The root DOM node
     * @return The article date in yyyy-mm-dd format
     */
    string extractDate(lxb_dom_node_t* root);

private:
    const string BASE_URL = "https://www.021.rs";

    /**
     * Recursively searches for article elements and extracts links
     * @param node Current lexbor node to search
     * @param links Vector to store found links
     */
    void findNewsLinks(lxb_dom_node_t* node, vector<string>& links);

    /**
     * Extracts the article title from the parsed HTML
     * @param root The root DOM node
     * @return The article title
     */
    string extractTitle(lxb_dom_node_t* root);

    /**
     * Extracts the article lead/subtitle from the parsed HTML
     * @param root The root DOM node
     * @return The article lead
     */
    string extractLead(lxb_dom_node_t* root);

    /**
     * Extracts the main article content from the parsed HTML
     * @param root The root DOM node
     * @return The main article content
     */
    string extractMainContent(lxb_dom_node_t* root);

    /**
     * Recursively extracts text content while filtering out ads and unwanted elements
     * @param node Current node to process
     * @param content Reference to string where content is accumulated
     */
    void extractContentText(lxb_dom_node_t* node, string& content);
};