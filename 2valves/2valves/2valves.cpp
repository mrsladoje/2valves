
#include "InformerScraper.h"
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

int main() {
    cout << "Fetching news from Informer.rs..." << endl;

	InformerScraper* scraper = new InformerScraper();
    // Fetch and extract news links directly from the website
    vector<string> links = scraper->fetchAndExtractNewsLinks();

    if (links.empty()) {
        cerr << "No news links found or failed to fetch content." << endl;
        return 1;
    }

    cout << "Found " << links.size() << " news links:" << endl;
    for (const auto& link : links) {
        cout << link << endl;
    }

    return 0;
}