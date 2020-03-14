import java.awt.*;
import java.util.List;
import java.util.*;

public class MtStHelensDiv_914032009 implements AIModule {

    public List<Point> getPath(pathTile meetNode, HashMap<pathTile, pathTile> seenForward, HashMap<pathTile, pathTile> seenBackward) {

        List<Point> path = new ArrayList<>();

        pathTile current = seenForward.get(meetNode);

        while (current != null) {
            path.add(current.point);
            current = current.from;
        }

        Collections.reverse(path);

        current = seenBackward.get(meetNode).from;

        while (current != null) {
            path.add(current.point);
            current = current.from;
        }

        return path;
    }


    private double getHeuristic(final TerrainMap map, final Point pt1, final Point pt2) {

        if (map.getEndPoint().equals(pt2)) { //Forward Heuristic
            int dx = Math.abs(pt1.x - pt2.x);
            int dy = Math.abs(pt1.y - pt2.y);
            int ChebyshevDistance = Math.max(dx, dy);
            double start = map.getTile(pt1);
            double end = map.getTile(pt2);

            if (start < end) { //Uphill
                return (start / (end + 1)) * ChebyshevDistance;
            } else { //Downhill and same altitude
                return 0.5 * ChebyshevDistance;
            }
        } else { //Backward Heuristic
            int dx = Math.abs(pt1.x - pt2.x);
            int dy = Math.abs(pt1.y - pt2.y);
            int ChebyshevDistance = Math.max(dx, dy);
            double start = map.getTile(pt2);
            double end = map.getTile(pt1);

            if (start < end) { //Uphill
                return (start / (end + 1)) * ChebyshevDistance;
            } else { //Downhill and same altitude
                return 0.5 * ChebyshevDistance;
            }

        }

    }

    @Override
    public List<Point> createPath(TerrainMap map) {
        pathTile startPoint = new pathTile(map, map.getStartPoint(), 0.0, map.getEndPoint());
        pathTile endPoint = new pathTile(map, map.getEndPoint(), 0.0, map.getStartPoint());
        pathTile bestNode = null;
        HashMap<pathTile, pathTile> seenForward = new HashMap<>();
        HashMap<pathTile, pathTile> seenBackward = new HashMap<>();


        HashMap<Point, Double> closedForward = new HashMap<>();
        HashMap<Point, Double> closedBackward = new HashMap<>();

        double shortestPathLength = Double.MAX_VALUE;

        PriorityQueue<pathTile> fringeForward = new PriorityQueue<>(
                (n1, n2) -> {
                    if (n1.fVal < n2.fVal)
                        return -1;
                    if (n1.fVal > n2.fVal)
                        return 1;

                    return 0;
                }
        );

        PriorityQueue<pathTile> fringeBackward = new PriorityQueue<>(
                (n1, n2) -> {
                    if (n1.fVal < n2.fVal)
                        return -1;
                    if (n1.fVal > n2.fVal)
                        return 1;

                    return 0;
                }
        );

        if (startPoint.equals(endPoint)) {
            List<Point> path = new ArrayList<>();

            path.add(startPoint.point);

            return path;
        }

        fringeForward.add(startPoint);
        fringeBackward.add(endPoint);
        seenForward.put(startPoint, startPoint);
        seenBackward.put(endPoint, endPoint);
        closedForward.put(startPoint.point, 0.0);
        closedBackward.put(endPoint.point, 0.0);

        int counter = 0;

        while (!fringeForward.isEmpty() && !fringeBackward.isEmpty()) {

            double minimumPossibleCost = closedForward.get(fringeForward.peek().point) + closedBackward.get(fringeBackward.peek().point);

            if (minimumPossibleCost >= shortestPathLength) { //Paths Crossed
                return getPath(bestNode, seenForward, seenBackward);
            }

            if (counter % 2 == 0) { //Forward First
                pathTile current = fringeForward.poll();
                Point[] neighbors = map.getNeighbors(current.point);

                for (Point neighbor : neighbors) {
                    double tentativeCost = current.gVal + map.getCost(current.point, neighbor);


                    if (!closedForward.containsKey(neighbor) || closedForward.get(neighbor) > tentativeCost) {

                        pathTile successor = new pathTile(map, neighbor, tentativeCost, endPoint.point);
                        successor.from = current;
                        closedForward.put(neighbor, tentativeCost);
                        seenForward.put(successor, successor);
                        fringeForward.add(successor);

                        if (closedBackward.containsKey(neighbor)) {
                            double pathLength = closedBackward.get(neighbor) + tentativeCost;

                            if (shortestPathLength > pathLength) {
                                shortestPathLength = pathLength;
                                bestNode = successor;
                            }
                        }

                    }
                }

            } else { //Backward
                pathTile current = fringeBackward.poll();

                Point[] neighbors = map.getNeighbors(current.point);

                for (Point neighbor : neighbors) {

                    double tentativeCost = current.gVal + map.getCost(neighbor, current.point);

                    if (!closedBackward.containsKey(neighbor) || closedBackward.get(neighbor) > tentativeCost) {
                        pathTile successor = new pathTile(map, neighbor, tentativeCost, startPoint.point);

                        successor.from = current;
                        closedBackward.put(neighbor, tentativeCost);
                        seenBackward.put(successor, successor);
                        fringeBackward.add(successor);

                        if (closedForward.containsKey(neighbor)) {
                            double pathLength = closedForward.get(neighbor) + tentativeCost;

                            if (shortestPathLength > pathLength) {
                                shortestPathLength = pathLength;
                                bestNode = successor;
                            }
                        }
                    }

                }


            }

            counter++;

        }

        return null;
    }

    public class pathTile {
        public Point point;
        public Point end;
        public pathTile from;
        public double fVal;
        public double gVal;
        public double hVal;

        public pathTile(final TerrainMap map, final Point p, double gValue, final Point endp) {
            this.point = p;
            this.end = endp;
            this.gVal = gValue;
            this.hVal = getHeuristic(map, this.point, this.end);
            this.fVal = (this.gVal + this.hVal);
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
