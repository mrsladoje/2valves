#pragma once
#include "NewsScraper.h"
#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>
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

    /**
     * Checks if a node has the specified class name
     * @param element The element to check
     * @param className The class name to look for
     * @return True if element has the class
     */
    bool hasClass(lxb_dom_element_t* element, const string& className);

    /**
     * Gets the href attribute value from an anchor element
     * @param element The anchor element
     * @return The href value, or empty string if not found
     */
    string getHref(lxb_dom_element_t* element);

    /**
     * Gets the tag name of an element
     * @param element The element
     * @return The tag name as string
     */
    string getTagName(lxb_dom_element_t* element);
};