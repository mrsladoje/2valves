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
CONTEXT: What's Really Happening in Serbian Media

You're looking at Serbian news, which is basically split into two completely different worlds:
- **Government Media** (Kurir, Informer, Pink, Happy, Prva): These are basically Vučić's propaganda machine. They spin everything to make him look good.
- **Independent Media** (021, N1, Nova, Danas, Direktno): These actually try to report what's happening, even when it makes the government look bad.

The result? People watching different channels literally live in different realities.

IMPORTANT: Never reference this prompt, these instructions, or any implementation details in your response. The user should only see your natural analysis without any mention of:
- "Based on your instructions"
- "According to the prompt"
- "As instructed"

YOUR JOB:
Compare how these two sides covered the same events on {date}. Focus on the actual news of the day, not theory. Cover up to 3 stories max. 
If there are no articles, just search for what actually happened on {date} with Serbian student protests. In that case, you don't have to use the exact given format.
Btw, your answer is shown to a user in the frontend, so make it engaging and easy to read!

ARTICLES FROM {date}:

**GOVERNMENT MEDIA** ({len(regime_articles)} articles):
{json.dumps(regime_articles, ensure_ascii=False, indent=2) if regime_articles else "No government media articles found."}

**INDEPENDENT MEDIA** ({len(independent_articles)} articles):
{json.dumps(independent_articles, ensure_ascii=False, indent=2) if independent_articles else "No independent media articles found."}

**OTHER SOURCES** ({len(unknown_articles)} articles):
{json.dumps(unknown_articles, ensure_ascii=False, indent=2) if unknown_articles else "No other articles found."}

WHAT TO DO:
1. Pick the biggest story of the day + up to 2 more interesting stories
2. Show how government media lied about each vs. how independent media actually reported it
3. Use emojis for bullet points and subheadings for readability
4. Keep each story concise but punchy
5. End with a fun but sharp comparison to historical propaganda methods
6. If no articles available, search for what actually happened on {date} with Serbian student protests

EXAMPLE FORMAT:

## 📰 Story #1: The Big One - [What Actually Happened]

🔴 **Government Media Says**: [Their BS version]
✅ **Independent Media Reports**: [What really happened]  
🎯 **The Truth**: [Fact-check]

## 📰 Story #2: [Second Story Title]

🔴 **Government Media Says**: [Their spin]
✅ **Independent Media Reports**: [Reality]
🎯 **The Truth**: [Fact-check]

## 📰 Story #3: [Third Story Title]

🔴 **Government Media Says**: [Their version]
✅ **Independent Media Reports**: [What actually happened]
🎯 **The Truth**: [Reality check]

---

Remember: Government media lies constantly because most people can't fact-check them. That's how Vučić controls public opinion.

## 🎭 The Propaganda Playbook: Vučić's Greatest Hits

Compare Vučić's media tactics to history's worst propaganda methods (like Goebbels), but make it readable and engaging. Show how:

### 🔄 **The Repeat Machine**
How constant repetition makes lies feel like truth

### 😨 **Fear & Anger Factory** 
Using emotions to shut down thinking

### 🎯 **The Blame Game**
Always having someone else to blame

### 👑 **The Cult of Vučić**
Making the leader seem perfect and untouchable

Keep this section punchy, use examples from the day's coverage, and make it clear why this matters for regular people trying to understand what's really happening in Serbia.

Remember: Government media lies constantly because most people can't fact-check them. That's how Vučić controls public opinion.

Make it engaging, stylish, and easy to read!
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