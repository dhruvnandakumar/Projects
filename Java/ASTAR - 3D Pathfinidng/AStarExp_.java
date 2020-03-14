import java.awt.*;
import java.util.List;
import java.util.*;

public class AStarExp_914032009 implements AIModule {


    private double getHeuristic(final TerrainMap map, final Point pt1, final Point pt2) {

        int dx = Math.abs(pt1.x - pt2.x);
        int dy = Math.abs(pt1.y - pt2.y);
        int ChebyshevDistance = Math.max(dx, dy);
        double start = map.getTile(pt1);
        double end = map.getTile(pt2);

        if (start > end) {
            return ChebyshevDistance * Math.pow(2.0, (end - start));
        } else {
            return ChebyshevDistance;
        }

    }

    private double penalty(Point start, Point end, Point current) {//TODO: Implement

        int esX = end.x - start.x;
        int esY = end.y - start.y;
        int scX = start.x - current.x;
        int scY = start.y - current.y;
        int ecX = end.x - current.x;
        int ecY = end.y - current.y;

        double esMag = Math.sqrt((esX * esX + esY * esY));
        double scMag = Math.sqrt((scX * scX + scY * scY));
        double ecMag = Math.sqrt((ecX * ecX + ecY * ecY));

        double s = (esMag + scMag + ecMag) / 2;

        double area = Math.sqrt((s) * (s - esMag) * (s - scMag) * (s - ecMag));

        double height = (2 * area) / esMag;

        double heightPenalty = 1;

        double retVal = 1;

        if (end.x == start.x && end.y == start.y) {
            return 1;
        }

        if (end.y > start.y) {
            if (end.x > start.x) {
                if (current.x < start.x && current.y < start.y) {
                    retVal = 2;
                }
            } else if (end.x == start.x) {
                if (current.x != start.x && current.y < start.y)
                    retVal = 2;
            } else {
                if (current.x > start.x && current.y < start.y) {
                    retVal = 2;
                }
            }
        } else if (end.y < start.y) {
            if (end.x > start.x) {
                if (current.x < start.x && current.y > start.y)
                    retVal = 2;
            } else if (end.x == start.x) {
                if (current.x != start.x && current.y > start.y)
                    retVal = 2;
            } else {
                if (current.x > start.x && current.y > start.y)
                    retVal = 2;
            }
        } else {
            if (end.x > start.x) {
                if (current.y != start.y && current.x < start.x)
                    retVal = 2;
            } else {
                if (current.y != start.y && current.x > start.x)
                    retVal = 2;
            }
        }

        return retVal * heightPenalty;
    }

    @Override
    public List<Point> createPath(TerrainMap map) {

        final pathTile StartPoint = new pathTile(map, map.getStartPoint(), 0.0);
        final pathTile EndPoint = new pathTile(map, map.getEndPoint(), 0.0);

        HashMap<Point, Double> gCosts = new HashMap<>();

        PriorityQueue<pathTile> fringe = new PriorityQueue<>(
                (n1, n2) -> {
                    if (n1.fVal < n2.fVal)
                        return -1;
                    if (n1.fVal > n2.fVal)
                        return 1;

                    return 0;
                }
        );

        fringe.add(StartPoint);

        //https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode

        while (!fringe.isEmpty()) {
            pathTile current = fringe.poll();

            if (current.equals(EndPoint)) {
                List<Point> path = new ArrayList<>();

                path.add(0, current.point);

                while (current.from != null) {
                    path.add(0, current.from.point);
                    current = current.from;
                }

                return path;
            }

            Point[] neighbors = map.getNeighbors(current.point);

            for (Point neighbor : neighbors) {
                double tentativeCost = current.gVal + map.getCost(current.point, neighbor);

                if (!gCosts.containsKey(neighbor) || gCosts.get(neighbor) > tentativeCost) {
                    pathTile successor = new pathTile(map, neighbor, tentativeCost);
                    successor.from = current;
                    gCosts.put(neighbor, tentativeCost);

                    fringe.add(successor);

                }

            }

        }

        return null;
    }

    public class pathTile {
        public Point point;
        public pathTile from;
        public double fVal;
        public double gVal;
        public double hVal;

        public pathTile(final TerrainMap map, final Point p, double gValue) {
            this.point = p;
            this.gVal = gValue;
            this.hVal = getHeuristic(map, this.point, map.getEndPoint());
            this.fVal = (this.gVal + this.hVal) /** penalty(map.getStartPoint(), map.getEndPoint(), this.point)*/;
            this.from = null;

        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            pathTile pathTile = (pathTile) o;
            return Objects.equals(point, pathTile.point);
        }

        @Override
        public int hashCode() {
            return Objects.hash(point);
        }
    }

}
