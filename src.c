#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 4 // Number of cities

// Function to find the minimum of two integers
int min(int a, int b) {
    return (a < b) ? a : b;
}

// Function to calculate the factorial of a number
int factorial(int n) {
    if (n == 0)
        return 1;
    else
        return n * factorial(n - 1);
}

// Function to calculate the cost of the current path
int calculateCost(int path[], int graph[N][N]) {
    int cost = 0;
    for (int i = 0; i < N - 1; i++) {
        cost += graph[path[i]][path[i + 1]];
    }
    cost += graph[path[N - 1]][path[0]]; // Return to the starting city
    return cost;
}

// Function to perform parallel depth-first search
void dfs(int graph[N][N], int path[], int visited[], int cost, int minCost, int depth) {
    if (depth == N) {
        minCost = min(minCost, cost);
        return;
    }

    for (int i = 0; i < N; i++) {
        if (!visited[i]) {
            visited[i] = 1;
            path[depth] = i;

            // Recursive call
            dfs(graph, path, visited, cost + graph[path[depth - 1]][i], minCost, depth + 1);

            visited[i] = 0; // Backtrack
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int graph[N][N] = {
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0}
    };

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Divide the cities among processes
    int citiesPerProcess = N / size;
    int startCity = rank * citiesPerProcess;
    int endCity = (rank + 1) * citiesPerProcess;
    if (rank == size - 1) {
        endCity = N;
    }

    // Initialize variables for tracking the best path and its cost
    int minCost = INT_MAX;
    int path[N];
    int visited[N] = {0};

    // Perform DFS for each subset of cities
    for (int i = startCity; i < endCity; i++) {
        visited[i] = 1;
        path[0] = i;
        dfs(graph, path, visited, 0, minCost, 1);
        visited[i] = 0;
    }

    // Gather results from all processes
    int allMinCosts[size];
    MPI_Gather(&minCost, 1, MPI_INT, allMinCosts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Process 0 finds the minimum cost among all results
    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            minCost = min(minCost, allMinCosts[i]);
        }
        printf("Minimum cost: %d\n", minCost);
    }

    MPI_Finalize();
    return 0;
}