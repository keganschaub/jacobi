import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.concurrent.Semaphore;

public class Main {

    //Global variables
    public static double[][] oldGrid, newGrid;
    public static int maxIters = 3000, maxGrid = 250, maxThread = 16;
    public static int gridSize = 0, threadSize = 0, numWorkers = 0,
            seconds = 0, microseconds = 0;
    public static int []arrive = new int[maxThread];
    public static double epsilon = 0.1;
    public static Semaphore mutex;

    public static void main(String[] args) throws FileNotFoundException, UnsupportedEncodingException {

        //Local variables
        double left = 1.0, top = 1.0, right = 1.0, bottom = 1.0;
        mutex = new Semaphore(1);

        //For writing to file "JacobiAnsJava"
        PrintWriter writer = new PrintWriter("JacobiAnsJava.txt", "UTF-8");

        //Needs at least gridSize, numWorkers
        if ((args.length - 1) < 2) {
            System.err.println("Too few arguments");
            System.exit(1);
        }

        //Assign variables to be commandline arguments
        for (int i = 0; i < (args.length - 1); i++) {
            switch (i) {
                case 0:
                    gridSize = Integer.parseInt(args[i]);
                    break;
                case 1:
                    numWorkers = Integer.parseInt(args[i]);
                    break;
                case 2:
                    left = Double.parseDouble(args[i]);
                    break;
                case 3:
                    top = Double.parseDouble(args[i]);
                    break;
                case 4:
                    right = Double.parseDouble(args[i]);
                    break;
                case 5:
                    bottom = Double.parseDouble(args[i]);
                    break;
                case 6:
                    epsilon = Double.parseDouble(args[i]);
                    break;
                default:
                    System.err.println("Too many arguments");
                    System.exit(1);
            }//switch
        }//for i

        //Set size of grids
        oldGrid = new double[maxGrid][maxGrid];
        newGrid = new double[maxGrid][maxGrid];
        //Worker threads
        Worker[] worker = new Worker[numWorkers];
        //What the size of each thread should be
        threadSize = gridSize / numWorkers;

        //Initialize grids
        for (int i = 0; i < (gridSize + 2); i++) {
            for (int j = 0; j < (gridSize + 2); j++) {
                if (i == 0) {
                    oldGrid[i][j] = top;
                    newGrid[i][j] = top;
                } else if (j == 0) {
                    oldGrid[i][j] = left;
                    newGrid[i][j] = left;
                } else if (i == (gridSize + 1)) {
                    oldGrid[i][j] = bottom;
                    newGrid[i][j] = bottom;
                } else if (j == (gridSize + 1)) {
                    oldGrid[i][j] = right;
                    newGrid[i][j] = right;
                } else {
                    oldGrid[i][j] = 0;
                    newGrid[i][j] = 0;
                }
            }//for j
        }//for i

        //Initialize arrive
        for (int i = 0; i < numWorkers; i++){
            arrive[i] = 0;
        }

        //create threads
        for (int i = 0; i < numWorkers; i++) {
            worker[i] = new Worker(i);
            worker[i].start();
        }//for i

        for (int i = 0; i < numWorkers; i++){
            try {
                worker[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        //Prints to standard out
        System.out.println("main: numProcs = " + numWorkers + ", N = " + gridSize);
        System.out.println("execution time = " + seconds + " seconds, " + microseconds + " microseconds");

        //Write to file
        writer.println("Grid     = " + gridSize + " x " + gridSize);
        writer.println("numProcs = " + numWorkers);
        writer.println("left     = " + left);
        writer.println("top      = " + top);
        writer.println("right    = " + right);
        writer.println("bottom   = " + bottom);
        writer.println("epsilon  = " + epsilon);
        writer.println("execution time: " + seconds + "seconds, " + microseconds + "microseconds");

        //Write final grid to file
        for (int i = 0; i < (gridSize + 2); i++){
            for (int j  = 0; j < (gridSize + 2); j++){
                writer.print("   " + oldGrid[i][j]);
            }//for j
            writer.println("");
        }//for i

        for (int i = 0; i < (gridSize + 2); i++){
            for (int j  = 0; j < (gridSize + 2); j++){
                System.out.print("   " + oldGrid[i][j]);
            }//for j
            System.out.println("");
        }//for i

    }//main method
}//Main class
