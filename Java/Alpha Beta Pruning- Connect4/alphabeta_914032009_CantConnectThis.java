import java.util.Collections;
import java.util.HashMap;
import java.util.Hashtable;

public class alphabeta_914032009_CantConnectThis extends AIModule{
    int player;
    int opponent;
    int maxDepth = 8;
    int bestMoveSeen;
    int moveOrder[] = {3,0,6,4,2,5,1};

    private static int[][] evaluationTable = {
            {3, 4, 5, 7, 5, 4, 3},
            {4, 6, 8, 10, 8, 6, 4},
            {5, 8, 11, 13, 11, 8, 5},
            {5, 8, 11, 13, 11, 8, 5},
            {4, 6, 8, 10, 8, 6, 4},
            {3, 4, 5, 7, 5, 4, 3}};


    private static int[][] leftLoopArguments = {
            {0,0,0,1,1,1,1},
            {0,0,0,1,1,1,1},
            {0,0,0,1,1,1,1},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}};

    private static int[][] rightLoopArguments = {
            {1,1,1,1,0,0,0},
            {1,1,1,1,0,0,0},
            {1,1,1,1,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}};

    public void getNextMove(final GameStateModule game)
    {
        player = game.getActivePlayer();
        opponent = (game.getActivePlayer() == 1?2:1);
        //begin recursion
        while(!terminate){
            alphaBeta(game, 0, player, Integer.MIN_VALUE, Integer.MAX_VALUE);
            if(!terminate)
                chosenMove = bestMoveSeen;
        }
        if(game.canMakeMove(chosenMove))
            game.makeMove(chosenMove);
    }

    private int alphaBeta(final GameStateModule state, int depth, int playerID, int alpha, int beta) {
        if (depth == maxDepth || terminate) {
            return zugzwang(state);
        }
        depth++;
        int value = 0;
        //max's turn
        int bestVal = Integer.MIN_VALUE;
        if(playerID == player){
            value = Integer.MIN_VALUE + 1;
            moveOrder = getMoveOrder(state,false);

            for(int i:moveOrder){
                if(state.canMakeMove(i)) {
                    state.makeMove(i);
                    value = Math.max(value, alphaBeta(state, depth, opponent, alpha, beta));
                    state.unMakeMove();

                    if(value >= beta){
                        if (value > bestVal){
                            if (depth == 1) { //top of recursion, make our move choice
                                bestMoveSeen = i;
                            }
                        }
                        return value;
                    }

                    alpha = Math.max(alpha,value);

                    if (value > bestVal){
                        bestVal = value;
                        if (depth == 1) { //top of recursion, make our move choice
                            bestMoveSeen = i;
                        }
                    }

                }
            }
        }

        else { //min's turn
            value = Integer.MAX_VALUE;
            moveOrder = getMoveOrder(state,true);

            for(int i:moveOrder) {
                if (state.canMakeMove(i)) {
                    state.makeMove(i);
                    value = Math.min(value, alphaBeta(state, depth, player, alpha, beta));
                    state.unMakeMove();

                    if(value <= alpha)
                        return value;

                    beta = Math.min(value, beta);

                }
            }
        }
        return value;
    }

    int[] getMoveOrder(GameStateModule state,boolean minimum){

        int[] moves = {0,0,0,0,0,0,0};


        for(int i =0 ; i<7; i++){
            if(state.canMakeMove(i)){
                state.makeMove(i);
                moves[i] = heuristicEval(state);
                state.unMakeMove();
            }
        }

        int[] order = {0,0,0,0,0,0,0};

        if(!minimum){ //Return in ascending order
            for(int j = 0; j<7;j++){
                int idx = 0;
                int val = Integer.MIN_VALUE;
                for(int i = 0; i<7; i++){
                    if(moves[i] > val){
                        val = moves[i];
                        idx = i;
                    }
                }
                moves[idx] = Integer.MIN_VALUE;
                order[j] = idx;
            }

        } else { //Return in descending order
            for(int j = 0; j<7;j++){
                int idx = 0;
                int val = Integer.MAX_VALUE;
                for(int i = 0; i<7; i++){
                    if(moves[i] < val){
                        val = moves[i];
                        idx = i;
                    }
                }
                moves[idx] = Integer.MAX_VALUE;
                order[j] = idx;
            }
        }

        return order;
    }

    int heuristicEval(GameStateModule state){
        int score = 0;

        for(int i = 0; i<6; i++){
            for(int j = 0; j<7; j++){
                if(state.getAt(j,i) == player){
                    score += evaluationTable[i][j];
                    if(i%2 == 0){
                        score += 3;
                    } else {
                        score += 2;
                    }

                } else if(state.getAt(j,i) == opponent){
                    score -= evaluationTable[i][j];
                    if(i%2 == 0){
                        score -= 1;
                    } else {
                        score -=3;
                    }
                }
            }
        }

        return score;
    }

    private int zugzwang(GameStateModule state){
        int score = 0;

        int [][] gameBoard = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};
        int [][] horizontalThreats = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};
        int [][] verticalThreats = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};
        int [][] leftDiagonalThreats = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};
        int [][] rightDiagonalThreats = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}};

        if(state.isGameOver()){
            if(state.getWinner() == player){
                return 1000;
            } else {
                return -1000;
            }
        }

        for(int i = 0; i<6; i++){
            for(int j = 0; j<7; j++){
                gameBoard[i][j] = state.getAt(j,i);
            }
        }

        for(int i = 0; i<6; i++){
            for(int j = 0; j<7; j++){

                if(gameBoard[i][j] == 0)
                    continue;

                int toMatch = gameBoard[i][j];

                //Match Horizontal -------------------------------------------------------------------------------------------------------------------------------------------
                if(j == 0){
                    if((gameBoard[i][1] == gameBoard[i][2]) && (gameBoard[i][1] == gameBoard[i][j])){
                        if(i == 0 && gameBoard[i][j+3] == 0){
                            horizontalThreats[i][j+3] = toMatch;
                        } else {
                            if(i != 0 && gameBoard[i-1][3] != 0 && gameBoard[i][3] == 0){
                                horizontalThreats[i][3] = toMatch;
                            }
                        }
                    }
                }

                if(j == 1){
                    for(int k = j-1; k<= j; k++){
                        horizontalThreatCount(gameBoard, horizontalThreats, i, toMatch, k);
                    }
                }

                if(j >= 2 && j <= 3){
                    for(int k = j-2; k< j; k++){
                        horizontalThreatCount(gameBoard, horizontalThreats, i, toMatch, k);
                    }
                }

                //Match Vertical --------------------------------------------------------------------------------------------------

                if(i >= 0 && i <= 2){
                    if(gameBoard[i+1][j] == gameBoard[i + 2][j] && gameBoard[i+1][j] == toMatch && gameBoard[i+3][j] == 0){
                        verticalThreats[i+3][j] = toMatch;
                    }
                }


            }
        } //Threat Analysis

        //Match Diagonal ----------------------------------------------------------------------------------------------------
        getLeftDiagonalThreats(gameBoard, leftDiagonalThreats);
        getRightDiagonalThreats(gameBoard, rightDiagonalThreats);


        score = updateFromTable(score,gameBoard);
        int numAdv = 0;
        int numT = 0;


        for(int i = 0; i<6; i++){
            for(int j = 0; j<7; j++){
                if(horizontalThreats[i][j] == player || verticalThreats[i][j] == player || leftDiagonalThreats[i][j] == player || rightDiagonalThreats[i][j] == player){
                    numAdv++;
                }
                else if(horizontalThreats[i][j] == opponent || verticalThreats[i][j] == opponent || leftDiagonalThreats[i][j] == opponent || rightDiagonalThreats[i][j] == opponent) {
                    numT++;
                }
            }
        }

        score += (numAdv * 120) + (-1 * numT * 100);

        return score;
    }

    private void getRightDiagonalThreats(int[][] gameBoard, int[][] diagonalThreats) {
        for(int i = 0; i<6; i++){
            for(int j = 0; j<7; j++){
                if(rightLoopArguments[i][j] == 0)
                    continue;

                if(gameBoard[i][j] == gameBoard[i+1][j+1] && gameBoard[i][j] == gameBoard[i+2][j+2] && gameBoard[i+3][j+3] == 0 && gameBoard[i+2][j+3] != 0)
                    diagonalThreats[i+3][j+3] = gameBoard[i][j];
            }
        }

    }

    private void getLeftDiagonalThreats(int[][] gameBoard, int[][] diagonalThreats) {

        for(int i = 0; i<6; i++){
            for(int j = 0; j < 7; j++){
                if(leftLoopArguments[i][j] == 0)
                    continue;

                if(gameBoard[i][j] == gameBoard[i+1][j-1] && gameBoard[i][j] == gameBoard[i+2][j-2] && gameBoard[i+3][j-3] == 0 && gameBoard[i+2][j-3] != 0){
                    diagonalThreats[i+3][j-3] = gameBoard[i][j];
                } //Diagonal match
            }
        }
    }


    private void horizontalThreatCount(int[][] gameBoard, int[][] horizontalThreats, int i, int toMatch, int k) {
        if((gameBoard[i][k] == gameBoard[i][k+1])&& (gameBoard[i][k+1] == toMatch) && (gameBoard[i][k+2] == gameBoard[i][k+1])){
            if((i == 0)){

                if( k != 0){
                    if(gameBoard[i][k-1] == 0 || gameBoard[i][k+3] == 0){

                        if(gameBoard[i][k-1] == 0)
                            horizontalThreats[i][k-1] = toMatch;

                        if(gameBoard[i][k+3] == 0)
                            horizontalThreats[i][k+3] = toMatch;

                    } else {
                        if(gameBoard[i][k+3] == 0) {
                            horizontalThreats[i][k+3] = toMatch;

                        }
                    }
                }


            } else {
                if(k != 0){
                    if((gameBoard[i-1][k-1] != 0 && gameBoard[i][k-1] == 0) || (gameBoard[i-1][k+3] != 0 && gameBoard[i][k+3] == 0)){

                        if((gameBoard[i-1][k-1] != 0 && gameBoard[i][k-1] == 0))
                            horizontalThreats[i][k-1] = toMatch;

                        if(gameBoard[i-1][k+3] != 0 && gameBoard[i][k+3] == 0)
                            horizontalThreats[i][k+3] = toMatch;


                    }
                }
                else {
                    if((gameBoard[i-1][k+3] != 0 && gameBoard[i][k+3] == 0)) {
                        horizontalThreats[i][k+3] = toMatch;
                    }
                }

            }
        }
    }

    private int updateFromTable(int score, int[][] gameBoard) {
        for(int i = 0; i<6;  i++){
            for(int j = 0; j<7; j++){
                if(gameBoard[i][j] == player){
                    score += evaluationTable[i][j];
                } else if(gameBoard[i][j] == opponent){
                    score -= evaluationTable[i][j];
                }
            }
        }

        return score;
    }
}
