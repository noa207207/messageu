#!/bin/bash
# MessageU Reset Script - Clean slate for testing

echo "🧹 Cleaning MessageU..."

# Stop any running servers
pkill -f "python3 server.py" 2>/dev/null || true

# Navigate to project directory
cd "$(dirname "$0")"

# Remove database
if [ -f server/defensive.db ]; then
    rm server/defensive.db
    echo "✅ Deleted server/defensive.db"
else
    echo "ℹ️  No database to delete"
fi

# Remove client credentials
if [ -f client/my.info ]; then
    rm client/my.info
    echo "✅ Deleted client/my.info"
else
    echo "ℹ️  No client credentials to delete"
fi

# Remove any backup files
rm -f client/my.info.* 2>/dev/null

echo ""
echo "✨ MessageU is now reset!"
echo ""
echo "📝 Next steps:"
echo "   1. Terminal 1: cd server && python3 server.py"
echo "   2. Terminal 2: cd client && ./messageu"
echo "   3. Terminal 3: cd client && ./messageu"
echo ""
echo "🎯 Register with fresh usernames!"

