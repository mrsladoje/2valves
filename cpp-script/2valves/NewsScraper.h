#pragma once
#include <string>
#include <vector>
#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

using std::string;
using std::vector;

/**
 * Abstract interface for news website scrapers
 */
class NewsScraper {
protected:
    const vector<string> PROTEST_KEYWORDS = {
        "protest", "blokad", "student", "šetnja", "demonstrac", "skupštin",
        "vučić", "vucic", "vlada", "vlast", "režim", "rezim", "sns", "opozicij",
        "policij", "žandarm", "zandar", "hapš", "haps", "suzavac",
        "štrajk", "strajk", "bojkot", "autoput", "saobraćaj", "saobracaj",
        "pokret", "aktivis", "građan", "gradan", "korupcij", "cenzur",
        "revolucij", "obojen", "ruš", "destabiliz", "ustaš",
        "ustas", "teroris", "idiot", "budal", "izdajn", "huligan",
        "ekstremis", "radikal", "fašis", "fasis", "ćac", "cac", "pionirsk",
        "izbor", "vanredn", "list", "saj", "rektor", "dekan", "fasizam",
        "pritvor", "nasil", "plenum", "vjt", "antisrp"
    };

    /**
     * Checks if a URL is relevant for protest-related news
     * @param url The URL to check
     * @return True if the URL contains protest-related keywords
     */
    bool isRelevantForProtests(const std::string& url) const;

    /**
     * Checks if a node has the specified class name
     * @param element The element to check
     * @param className The class name to look for
     * @return True if element has the class
     */
    bool hasClass(lxb_dom_element_t* element, const string& className) const;

    /**
     * Gets the href attribute value from an anchor element
     * @param element The anchor element
     * @return The href value, or empty string if not found
     */
    string getHref(lxb_dom_element_t* element) const;

    /**
     * Gets the tag name of an element
     * @param element The element
     * @return The tag name as string
     */
    string getTagName(lxb_dom_element_t* element) const;

    /**
     * Check if element has a specific attribute with a specific value
     * @param element The DOM element to check
     * @param attributeName The attribute name to check for
     * @param attributeValue The expected attribute value
     * @return True if element has the attribute with the specified value
     */
    bool hasAttribute(lxb_dom_element_t* element, const string& attributeName, const string& attributeValue) const;

	/**
	 * Check if element has a specific ID
	 * @param element The DOM element to check
	 * @param id The expected ID value
	 * @return True if element has the specified ID
	 */
    bool hasId(lxb_dom_element_t* element, const string& id) const;

    /**
     * Cleans and normalizes text content by removing extra whitespace
     * @param text The text to clean
     * @return Cleaned text
     */
    string cleanTextContent(const string& text) const;

    /**
     * Finds the first element with the specified class name
     * @param root The root node to search from
     * @param className The class name to search for
     * @return Pointer to the element, or nullptr if not found
     */
    lxb_dom_element_t* findElementByClass(lxb_dom_node_t* root, const string& className) const;

    /**
     * Finds the first element with the specified class name and tag name
     * @param root The root node to search from
     * @param className The class name to search for
     * @param tagName The tag name to search for
     * @return Pointer to the element, or nullptr if not found
     */
    lxb_dom_element_t* findElementByClassAndTag(lxb_dom_node_t* root, const string& className, const string& tagName) const;

    /**
     * Finds the first element with the specified tag name
     * @param root The root node to search from
     * @param tagName The tag name to search for
     * @return Pointer to the element, or nullptr if not found
     */
    lxb_dom_element_t* findElementByTag(lxb_dom_node_t* root, const string& tagName) const;

    /**
     * Gets the text content of an element
     * @param element The element to get text from
     * @return The text content as string
     */
    string getElementTextContent(lxb_dom_element_t* element) const;

public:
    virtual ~NewsScraper() = default;

    /**
     * Fetches and extracts news article links from the website
     * @param url The URL to scrape (uses default if empty)
     * @return Vector of complete URLs to news articles
     */
    virtual vector<string> fetchAndExtractNewsLinks() = 0;

    /**
     * Extracts news article links from already fetched HTML content
     * @param htmlContent The HTML content as string
     * @return Vector of complete URLs to news articles
     */
    virtual vector<string> extractNewsLinks(const string& htmlContent) = 0;

    /**
     * Fetches and extracts the full content of a news article
     * @param url The URL of the article to scrape
     * @return The article content as a formatted string, or empty string on error
     */
    virtual string fetchAndExtractArticleContent(const string& url) = 0;
};



