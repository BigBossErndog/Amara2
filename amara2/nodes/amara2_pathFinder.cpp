namespace Amara {
    struct PathPoint {
        std::string id;

        std::vector<Amara::PathPoint*> connections;

        void connect(Amara::PathPoint* point) {
            connections.push_back(point);
        }

        void connect(int index, Amara::PathPoint* point) {
            if (connections.size() <= index) {
                connections.resize(index + 1, nullptr);
            } 
            connections[index] = point;
        }

        bool connected(Amara::PathPoint* _point) {
            for (Amara::PathPoint* point: connections) {
                if (point == _point) return true;
            }
            return false;
        }
        bool connected(int index) {
            if (index < 0 || index >= connections.size()) return false;
            if (connections[index] != nullptr) return true;
            return false;
        }
    };

    class PathFinder: public Amara::Node {
    public:
        int mapWidth = -1;
        int mapHeight = -1;

        std::unordered_map<std::string, Amara::PathPoint*> pathMap;
        std::vector<Amara::PathPoint> pathPoints;

        PathFinder(): Amara::Node() {
            set_base_node_id("PathFinder");
        }

        Amara::PathPoint* createPoint() {
            Amara::PathPoint newPoint;

            pathPoints.push_back(newPoint);

            return &pathPoints.back();
        }

        Amara::PathPoint* getPoint(int index) {
            if (index < 0 || index >= pathPoints.size()) return nullptr;
            return &pathPoints[index];
        }

        Amara::PathPoint* getPoint(int _x, int _y) {
            if (mapWidth == -1 || mapHeight == -1) return nullptr;
            if (_x < 0 || _y < 0) return nullptr;
            if (_x >= mapWidth || _y >= mapHeight) return nullptr;
            
            int index = _y*mapWidth + _x;
            return getPoint(index);
        }

        static void bind_lua(sol::state& lua) {
            lua.new_usertype<Amara::PathFinder>("PathFinder",
                sol::base_classes, sol::bases<Amara::Node>(),
                "mapWidth", &Amara::PathFinder::mapWidth,
                "mapHeight", &Amara::PathFinder::mapHeight,
                "getPoint", sol::overload(
                    sol::resolve<Amara::PathPoint*(int)>(&Amara::PathFinder::getPoint),
                    sol::resolve<Amara::PathPoint*(int, int)>(&Amara::PathFinder::getPoint)
                ),
                "createPoint", &Amara::PathFinder::createPoint
            );
        }
    };
}