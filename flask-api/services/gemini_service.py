import logging
import json
import requests
from typing import List, Dict, Any
from datetime import datetime
from bson import ObjectId

logger = logging.getLogger(__name__)

class GeminiService:
    """Service for interacting with Google Gemini API"""
    
    def __init__(self, config):
        self.config = config
        self.api_key = config.GEMINI_API_KEY
        self.model = config.GEMINI_MODEL
        self.base_url = "https://generativelanguage.googleapis.com/v1beta"
        
        if not self.api_key:
            logger.error("GEMINI_API_KEY not found in environment variables")
            raise ValueError("Gemini API key is required")
    
    def _serialize_articles_for_json(self, articles: List[Dict]) -> List[Dict]:
        """Convert articles to JSON-serializable format"""
        serialized_articles = []
        
        for article in articles:
            serialized_article = {}
            for key, value in article.items():
                if isinstance(value, datetime):
                    serialized_article[key] = value.isoformat()
                elif isinstance(value, ObjectId):
                    serialized_article[key] = str(value)
                else:
                    serialized_article[key] = value
            serialized_articles.append(serialized_article)
        
        return serialized_articles
    
    def _classify_news_source(self, article_url: str) -> str:
        """Classify news source as regime or independent based on URL"""
        url_lower = article_url.lower()
        
        for source in self.config.REGIME_SOURCES:
            if source in url_lower:
                return "regime"
        
        for source in self.config.INDEPENDENT_SOURCES:
            if source in url_lower:
                return "independent"
        
        return "unknown"
    
    def _create_analysis_prompt(self, date: str, articles: List[Dict]) -> str:
        """Create the analysis prompt for Gemini"""
        
        # Classify articles by source type
        regime_articles = []
        independent_articles = []
        unknown_articles = []
        
        # Serialize articles to handle datetime objects
        serialized_articles = self._serialize_articles_for_json(articles)
        
        for article in serialized_articles:
            classification = self._classify_news_source(article.get('_id', ''))
            if classification == "regime":
                regime_articles.append(article)
            elif classification == "independent":
                independent_articles.append(article)
            else:
                unknown_articles.append(article)
        
        prompt = f"""
CONTEXT: Serbian Media Landscape Analysis

You are analyzing the Serbian media landscape, which is characterized by a stark division between regime-controlled media and independent outlets. This division creates what can be described as "parallel realities" in news coverage.

HISTORICAL CONTEXT:
The current Serbian media landscape bears disturbing similarities to historical propaganda techniques, particularly those employed by Joseph Goebbels during Nazi Germany. The regime-controlled media in Serbia employs similar methods:

1. **Repetition of False Narratives**: Constant repetition of government talking points until they become accepted as truth
2. **Emotional Manipulation**: Using fear, anger, and nationalism to override rational thinking
3. **Demonization of Opposition**: Systematic character assassination of political opponents and independent journalists
4. **Control of Information Flow**: Monopolizing prime-time slots and most-read publications
5. **Scapegoating**: Blaming external forces (EU, opposition, NGOs) for all problems
6. **Cult of Personality**: Excessive praise and protection of the leader (Aleksandar Vučić)

MEDIA CLASSIFICATION:
- **Regime-Controlled Media** (Kurir, Informer, Pink, Happy, Prva): These outlets function as propaganda arms of the ruling party. They consistently promote government narratives, attack opposition figures, and rarely engage in critical journalism.
- **Independent Media** (021, N1, Nova, Beta, Danas): These outlets attempt to maintain journalistic standards, report critically on government actions, and provide platforms for diverse viewpoints.

ANALYSIS TASK:
Analyze the news coverage for {date} and compare how these two types of media covered the same events, particularly focusing on:

1. **Narrative Framing**: How do regime vs. independent media frame the same events?
2. **Language and Tone**: Identify propaganda techniques vs. journalistic reporting
3. **Information Gaps**: What stories does each type of media emphasize or ignore?
4. **Student Protests Context**: Pay special attention to coverage of student protests and government response
5. **Manipulation Techniques**: Identify specific Goebbels-style propaganda methods being used

ARTICLES FROM {date}:

REGIME-CONTROLLED SOURCES ({len(regime_articles)} articles):
{json.dumps(regime_articles, ensure_ascii=False, indent=2) if regime_articles else "No regime media articles found for this date."}

INDEPENDENT SOURCES ({len(independent_articles)} articles):
{json.dumps(independent_articles, ensure_ascii=False, indent=2) if independent_articles else "No independent media articles found for this date."}

UNCLASSIFIED SOURCES ({len(unknown_articles)} articles):
{json.dumps(unknown_articles, ensure_ascii=False, indent=2) if unknown_articles else "No unclassified articles found."}

INSTRUCTIONS:
1. If articles are available, analyze the stark differences in coverage between regime and independent media
2. If no articles are available for this date, perform a Google search about student protests in Serbia on {date} and provide analysis based on what you find
3. Highlight specific examples of propaganda techniques
4. Draw parallels to historical propaganda methods where applicable
5. Conclude with observations about the state of media freedom in Serbia

Please provide a comprehensive analysis that exposes the "parallel realities" created by these different media ecosystems.
"""
        
        return prompt
    
    def analyze_news_coverage(self, date: str, articles: List[Dict]) -> Dict[str, Any]:
        """Analyze news coverage using Gemini API"""
        try:
            prompt = self._create_analysis_prompt(date, articles)
            
            # Prepare request to Gemini API
            url = f"{self.base_url}/models/{self.model}:generateContent"
            headers = {
                "Content-Type": "application/json",
            }
            
            payload = {
                "contents": [{
                    "parts": [{
                        "text": prompt
                    }]
                }],
                "generationConfig": {
                    "temperature": 0.7,
                    "maxOutputTokens": 4000,
                    "topP": 0.9,
                    "topK": 40
                }
            }
            
            # Make request
            response = requests.post(
                f"{url}?key={self.api_key}",
                headers=headers,
                json=payload,
                timeout=60
            )
            
            if response.status_code != 200:
                logger.error(f"Gemini API error: {response.status_code} - {response.text}")
                return {
                    "error": f"API request failed with status {response.status_code}",
                    "details": response.text
                }
            
            result = response.json()
            
            # Extract the analysis text
            if "candidates" in result and len(result["candidates"]) > 0:
                analysis_text = result["candidates"][0]["content"]["parts"][0]["text"]
                
                # Classify articles for statistics
                regime_count = len([a for a in articles if self._classify_news_source(a.get('_id', '')) == "regime"])
                independent_count = len([a for a in articles if self._classify_news_source(a.get('_id', '')) == "independent"])
                
                return {
                    "date": date,
                    "analysis": analysis_text,
                    "statistics": {
                        "total_articles": len(articles),
                        "regime_articles": regime_count,
                        "independent_articles": independent_count,
                        "unknown_sources": len(articles) - regime_count - independent_count
                    },
                    "source_classification": {
                        "regime_sources": self.config.REGIME_SOURCES,
                        "independent_sources": self.config.INDEPENDENT_SOURCES
                    }
                }
            else:
                logger.error("Unexpected response format from Gemini API")
                return {"error": "Unexpected response format from API"}
            
        except requests.exceptions.Timeout:
            logger.error("Gemini API request timed out")
            return {"error": "API request timed out"}
        except requests.exceptions.RequestException as e:
            logger.error(f"Network error calling Gemini API: {str(e)}")
            return {"error": f"Network error: {str(e)}"}
        except Exception as e:
            logger.error(f"Error analyzing news coverage: {str(e)}")
            return {"error": f"Analysis failed: {str(e)}"}