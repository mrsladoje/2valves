from flask import Blueprint, jsonify

api_bp = Blueprint('api', __name__)

def register_api_routes(app, processor):
    """Register API routes"""
    
    @api_bp.route('/articles')
    def get_articles():
        """Get all articles from database"""
        try:
            articles = processor.get_all_articles()
            return jsonify({
                "count": len(articles),
                "articles": articles
            })
        except Exception as e:
            return jsonify({"error": str(e)}), 500

    @api_bp.route('/articles/<article_id>')
    def get_article(article_id):
        """Get specific article by ID"""
        try:
            article = processor.get_article_by_id(article_id)
            if article:
                return jsonify(article)
            else:
                return jsonify({"error": "Article not found"}), 404
        except Exception as e:
            return jsonify({"error": str(e)}), 500

    @api_bp.route('/articles/date/<date>')
    def get_articles_by_date(date):
        """Get all articles for a specific date (YYYY-MM-DD format)"""
        try:
            articles = processor.get_articles_by_date(date)
            if articles is None:
                return jsonify({"error": "Invalid date format. Use YYYY-MM-DD"}), 400
            
            return jsonify({
                "date": date,
                "count": len(articles),
                "articles": articles
            })
        except Exception as e:
            return jsonify({"error": str(e)}), 500

    @api_bp.route('/build')
    def build_cpp():
        """Manually trigger C++ project build"""
        try:
            success = processor.build_cpp_project()
            return jsonify({
                "success": success,
                "message": "Build completed successfully" if success else "Build failed"
            })
        except Exception as e:
            return jsonify({"error": str(e)}), 500

    @api_bp.route('/run-now')
    def run_now():
        """Manually trigger article processing"""
        try:
            success = processor.process_articles()
            return jsonify({
                "success": success,
                "message": "Job completed successfully" if success else "Job failed"
            })
        except Exception as e:
            return jsonify({"error": str(e)}), 500

    @api_bp.route('/stats')
    def get_stats():
        """Get database statistics"""
        try:
            stats = processor.get_stats()
            return jsonify(stats)
        except Exception as e:
            return jsonify({"error": str(e)}), 500
    
    app.register_blueprint(api_bp)