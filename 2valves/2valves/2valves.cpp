
#include "InformerScraper.h"
#include "KurirScraper.h"
#include "Scraper021.h"
#include "N1Scraper.h"
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


    cout << "Fetching news from Kurir.rs..." << endl;

    KurirScraper* scraperK = new KurirScraper();
    // Fetch and extract news links directly from the website
    links = scraperK->fetchAndExtractNewsLinks();

    if (links.empty()) {
        cerr << "No news links found or failed to fetch content." << endl;
        return 1;
    }

    cout << "Found " << links.size() << " news links:" << endl;
    for (const auto& link : links) {
        cout << link << endl;
    }

    Scraper021* scraper0 = new Scraper021();
    // Fetch and extract news links directly from the website
    links = scraper0->fetchAndExtractNewsLinks();

    if (links.empty()) {
        cerr << "No news links found or failed to fetch content." << endl;
        return 1;
    }

    cout << "Found " << links.size() << " news links:" << endl;
    for (const auto& link : links) {
        cout << link << endl;
    }

    N1Scraper* scraperN = new N1Scraper();
    // Fetch and extract news links directly from the website
    links = scraperN->fetchAndExtractNewsLinks();

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