from flask import Blueprint, jsonify

health_bp = Blueprint('health', __name__)

def register_health_routes(app, scheduler_service):
    """Register health check routes"""
    
    @health_bp.route('/')
    def home():
        """Health check endpoint"""
        return jsonify({
            "status": "running",
            "message": "Article scraper API is active",
            "next_run": scheduler_service.get_next_run_time()
        })
    
    app.register_blueprint(health_bp)