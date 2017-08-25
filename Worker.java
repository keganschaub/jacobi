/**
 * Created by kegan on 3/9/15.
 */
public class Worker extends Thread {
    int id;

    //Constructor
    public Worker(int id){
        this.id = id;
    }

    //run thread
    public void run(){
        int firstRow, lastRow;
        double maxdiff = 0.0, temp;

        //start and end of thread
        firstRow = id * Main.threadSize +1;
        lastRow = firstRow + Main.threadSize -1;

        barrier();
        //Iterate until value is determined
        for (int iters = 1; iters <= Main.maxIters; iters += 2){
            for (int i = firstRow; i <= Main.gridSize; i++){
                for (int j = 1; j <= Main.gridSize; j++){
                    Main.newGrid[i][j] = (Main.oldGrid[i-1][j] + Main.oldGrid[i+1][j] + Main.oldGrid[i][j-1] + Main.oldGrid[i][j+1]) * 0.25;
                }//for j
            }//for i
            barrier();
            for (int i = firstRow; i <= lastRow; i++){
                for (int j = 1; j <= Main.gridSize; j++){
                    Main.oldGrid[i][j] = (Main.newGrid[i-1][j] + Main.newGrid[i+1][j] + Main.newGrid[i][j-1] + Main.newGrid[i][j+1]) * 0.25;
                }
            }
            barrier();
        }//for iters

        barrier();
    }//run method

    private void barrier(){

        int stage = 1;

        while (stage < Main.numWorkers){
            Main.arrive[id]++;
            try {
                Main.mutex.acquire();
                while(!(Main.arrive[((id + stage) % Main.numWorkers)] >= Main.arrive[id])){
                    Main.mutex.release();
                    Thread.sleep(1);
                    Main.mutex.acquire();
                }
                Main.mutex.release();
            } catch (InterruptedException e) {};
            stage *= 2;

        }

    }
}//Worker class
