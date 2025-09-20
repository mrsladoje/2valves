#include "ParallelWorker.h"
#include <iostream>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

using json = nlohmann::json;

ParallelWorker::ParallelWorker() {}

ParallelWorker::Article ParallelWorker::processUrl(ScraperType type, const std::string& url) {
    Article result;
    result.url = url;

    auto scraper = NewsScraperFactory::createScraper(type);
    if (!scraper) return result;

    std::string content = scraper->fetchAndExtractArticleContent(url);
    result.text = content;

    result.date = content.substr(0, 10);

    return result;
}

json ParallelWorker::run() {
    using Source = std::pair<ScraperType, std::string>;
    std::vector<Source> scrapers = {
        {ScraperType::INFORMER, "Informer.rs"},
        {ScraperType::KURIR, "Kurir.rs"},
        {ScraperType::SCRAPER_021, "021.rs"},
        {ScraperType::N1, "N1info.rs"}
    };

    tbb::concurrent_vector<json> results;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, scrapers.size()),
        [&](const tbb::blocked_range<size_t>& range) {

            for (size_t i = range.begin(); i < range.end(); ++i) {
                auto [scraperType, siteName] = scrapers[i];
                std::unique_ptr<NewsScraper> scraper = NewsScraperFactory::createScraper(scraperType);

                auto links = scraper->fetchAndExtractNewsLinks();

                // Inner parallel_for: across links
                tbb::parallel_for(tbb::blocked_range<size_t>(0, links.size()),
                    [&](const tbb::blocked_range<size_t>& linkRange) {
                        for (size_t j = linkRange.begin(); j < linkRange.end(); ++j) {
                            auto article = processUrl(scraperType, links[j]);
                            json obj = {
                                {"_id", article.url},
                                {"date", article.date},
                                {"articleText", article.text}
                            };
                            results.push_back(obj);
                        }
                    }
                );
            }
        }
    );

    // Gather results into a single JSON array
    json output = json::array();
    for (auto& r : results) {
        output.push_back(r);
    }
    return output;
}
