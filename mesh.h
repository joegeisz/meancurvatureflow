#ifndef MESH_H
#define MESH_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "Eigen/Core"
#include "Eigen/Dense"
#include "Eigen/Sparse"
using namespace std;


class Mesh{
    public:
        Mesh();
        Mesh(string file);
        void displayPts(int mode);
        void displayTris(int mode);
        void update_pts();
        void reset_pts();
        std::clock_t last_update;
        std::clock_t last_reset;
    private:
        Eigen::MatrixXd pts;
        Eigen::MatrixXd edges;
        Eigen::MatrixXi tris;
        Eigen::SparseMatrix<int> A0; //npts by nedges, 1 if point in edge
        Eigen::SparseMatrix<int> A1; // nedges by nfaces, 1 if edge in face
        Eigen::SparseMatrix<int> A0A1; // npts by nfaces, 1 if point in face
        Eigen::SparseMatrix<int> A; // npts by npts, 1 if (pointa,pointb) is an edge (unsigned adjacency matrix)
        Eigen::SparseMatrix<int> GL; // Graph Laplacian: npts by npts = B0 - D
        Eigen::SparseMatrix<int> D; // Diagonal degree matrix
        Eigen::SparseMatrix<float> Laplacian();
        Eigen::SparseMatrix<float> rho;
        void set_points();
        void set_tris();
        void set_rho();
        void set_geometry();
        void set_B0();
        void set_A();
        void set_A0();
        void set_A1();
        void set_D_GL();
        float face_area(int face_idx);
        float vertex_dual_area(int pt_idx);
        int npts;
        int nedges;
        int ntris;
        string filename;

};


#endif