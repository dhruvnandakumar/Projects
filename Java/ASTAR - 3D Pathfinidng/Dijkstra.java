import java.awt.*;
import java.util.*;
import java.util.List;

public class Dijkstra implements AIModule{
    private double getHeuristic(final TerrainMap map, final Point pt1, final Point pt2)
    {
        return 0;

    }

    public class GraphNode {
        public Point point;
        public Dijkstra.GraphNode from;
        public double fVal;
        public double gVal;
        public double hVal;

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            Dijkstra.GraphNode graphNode = (Dijkstra.GraphNode) o;
            return Objects.equals(point, graphNode.point);
        }

        @Override
        public int hashCode() {
            return Objects.hash(point);
        }

        public GraphNode(final TerrainMap map, final Point p, double gValue){
            this.point = p;
            this.gVal = gValue;
            this.hVal = getHeuristic(map, this.point, map.getEndPoint());
            this.fVal = (this.gVal + this.hVal);
            this.from = null;
        }
    }

    public java.util.List<Point> createPath(final TerrainMap map){

        final Dijkstra.GraphNode StartPoint = new Dijkstra.GraphNode(map, map.getStartPoint(), 0.0);
        final Dijkstra.GraphNode EndPoint = new Dijkstra.GraphNode(map, map.getEndPoint(), 0.0);

        HashMap<Point, Double> gCosts = new HashMap<>();


        PriorityQueue<Dijkstra.GraphNode> fringe = new PriorityQueue<>(
                (n1, n2) -> {
                    if(n1.fVal < n2.fVal)
                        return -1;
                    if(n1.fVal > n2.fVal)
                        return 1;

                    return 0;
                }
        );

        fringe.add(StartPoint);

        while(!fringe.isEmpty()){
            Dijkstra.GraphNode current = fringe.poll();

            if(current.equals(EndPoint)){
                List<Point> path = new ArrayList<>();

                path.add(0, current.point);

                while(current.from != null){
                    path.add(0, current.from.point);
                    current = current.from;
                }

                return path;
            }

            Point[] neighbors = map.getNeighbors(current.point);

            for(Point neighbor : neighbors){
                double tentativeCost = current.gVal + map.getCost(current.point, neighbor);

                if(!gCosts.containsKey(neighbor) || gCosts.get(neighbor) > tentativeCost){
                    Dijkstra.GraphNode successor = new Dijkstra.GraphNode(map, neighbor, tentativeCost);
                    successor.from = current;
                    gCosts.put(neighbor, tentativeCost);

                    fringe.remove(successor);
                    fringe.add(successor);

                }

            }

        }

        return null;
    }
}
