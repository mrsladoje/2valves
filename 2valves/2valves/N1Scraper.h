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

    /**
     * Fetches and extracts the full content of a news article from N1info.rs
     * @param url The URL of the article to scrape
     * @return The article content as a formatted string, or empty string on error
     */
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

    /**
     * Extracts the article title from the parsed HTML
     * @param root The root DOM node
     * @return The article title
     */
    string extractTitle(lxb_dom_node_t* root);

    /**
     * Extracts the article lead text from the parsed HTML
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
     * Extracts content from article-content-wrapper elements
     * @param node Current node to process
     * @param content Reference to string where content is accumulated
     */
    void extractArticleContentWrappers(lxb_dom_node_t* node, string& content);

    /**
     * Extracts content from live blog entries
     * @param node Current node to process
     * @param content Reference to string where content is accumulated
     */
    void extractLiveBlogEntries(lxb_dom_node_t* node, string& content);

    /**
     * Finds the first element with the specified attribute and value
     * @param root The root node to search from
     * @param attributeName The attribute name to search for
     * @param attributeValue The attribute value to search for
     * @return Pointer to the element, or nullptr if not found
     */
    lxb_dom_element_t* findElementByAttribute(lxb_dom_node_t* root, const string& attributeName, const string& attributeValue);

    /**
     * Extracts the article published date from the parsed HTML
     * @param root The root DOM node
     * @return The article date in yyyy-mm-dd format
     */
    string extractDate(lxb_dom_node_t* root);
};