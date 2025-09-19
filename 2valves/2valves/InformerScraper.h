#pragma once

#include <string>
#include <vector>
#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>
#include <cpr/cpr.h>

using std::string;
using std::vector;

class InformerScraper {
public:
    /**
     * Fetches and extracts news article links from Informer.rs
     * @param url The URL to scrape (defaults to Informer.rs latest news)
     * @return Vector of complete URLs to news articles
     */
    static vector<string> fetchAndExtractNewsLinks(const string& url = "https://informer.rs/najnovije-vesti");

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    static vector<string> extractNewsLinks(const string& htmlContent);

private:
    static const string BASE_URL;

    /**
     * Fetches HTML content from a URL
     * @param url The URL to fetch
     * @return HTML content as string, or empty string on error
     */
    static string fetchHtmlContent(const string& url);

    /**
     * Recursively searches for news-item-title elements and extracts links
     * @param node Current lexbor node to search
     * @param links Vector to store found links
     */
    static void findNewsLinks(lxb_dom_node_t* node, vector<string>& links);

    /**
     * Checks if a node has the specified class name
     * @param element The element to check
     * @param className The class name to look for
     * @return True if element has the class
     */
    static bool hasClass(lxb_dom_element_t* element, const string& className);

    /**
     * Gets the href attribute value from an anchor element
     * @param element The anchor element
     * @return The href value, or empty string if not found
     */
    static string getHref(lxb_dom_element_t* element);

    /**
     * Gets the tag name of an element
     * @param element The element
     * @return The tag name as string
     */
    static string getTagName(lxb_dom_element_t* element);
};
