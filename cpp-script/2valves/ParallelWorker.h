#pragma once

#include "NewsScraperFactory.h"
#include <nlohmann/json.hpp>
#include <tbb/concurrent_vector.h>
#include <tbb/flow_graph.h>
#include <vector>
#include <string>

class ParallelWorker {
public:
    ParallelWorker();

    /**
     * Run the pipeline to fetch articles from all scrapers
     * @return JSON array of results
     */
    nlohmann::json run();

private:
    struct Article {
        std::string url;
        std::string date;
        std::string text;
    };

    /**
     * Processes one URL into an Article
     */
    Article processUrl(ScraperType type, const std::string& url);
};
