#include "mesh.h"
#include <GLUT/glut.h>
#include <cstdlib>
#include <ctime> 
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include "Eigen/Core"
#include "Eigen/SparseCore"
#include "Eigen/Dense"
#include "Eigen/Sparse"
using namespace std;


float generateRandomFloat() {
    
    // Generate a random float between 0 and 1
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float generateRandomInt(int max){
    float multiplier = static_cast<float>(max) / static_cast<float>(RAND_MAX);
    float num = static_cast<float>(std::rand())*multiplier;
    return floor(num);
}

bool isVertexLine(string line){
    if(line.at(0) == 'v' && line.at(1) == ' ' ){return true;}
    else{return false;}
}

bool isFaceLine(string line){
    if(line.at(0) == 'f'){return true;}
    else{return false;}
}

std::array<float,3> objFileVertexLine(string line){
    std::array<float,3> point;
    line = line.substr(2);
    int s1 = line.find(" ");
    int s2 = line.rfind(" ");
    string a = line.substr(0,s1);
    string b = line.substr(s1,s2-s1);
    string c = line.substr(s2);
    point[0] = stof(a);
    point[1] = stof(b);
    point[2] = stof(c);
    return point;
}

std::array<int,3> objFileFaceLine(string line){
    std::array<int,3> tri;
    line = line.substr(2);
    int s1 = line.find(" ");
    int s2 = line.rfind(" ");
    string a = line.substr(0,s1);
    string b = line.substr(s1,s2-s1);
    string c = line.substr(s2);
    tri[0] = stoi(a);
    tri[1] = stoi(b);
    tri[2] = stoi(c);
    return tri;
}

Mesh::Mesh()
{
    npts = 10;
    set_points();
    
}

Mesh::Mesh(string file)
{
    filename = file;
    ifstream ObjFile(filename);
    npts = 0;
    ntris = 0;
    string line;
    while (getline(ObjFile, line)) {
        if(isVertexLine(line)){npts++;}
        else if(isFaceLine(line)){ntris++;}
    }
    ObjFile.close();
    set_points();
    set_tris();
    set_rho();
    set_A();
    set_A0();
    set_A1();
    set_D_GL();
    last_update = std::clock();
}

Eigen::SparseMatrix<float> Mesh::Laplacian()
{
    Eigen::SparseMatrix<float> L;
    return L;
}

void Mesh::set_points()
{
    string line;
    ifstream ObjFile(filename);
    int i = 0;
    pts.resize(npts,3);
    while (getline(ObjFile, line)) {
        if(isVertexLine(line)){
            std::array<float,3> pt = objFileVertexLine(line);
            pts(i,0) = pt[0];
            pts(i,1) = pt[1];
            pts(i,2) = pt[2];
            i++;
        }
    }
    ObjFile.close();
};

void Mesh::set_tris(){
    string line;
    ifstream ObjFile(filename);
    int i = 0;
    tris.resize(ntris,3);
    while (getline(ObjFile, line)) {
        if(isFaceLine(line)){
            std::array<int,3> tri = objFileFaceLine(line);
            tris(i,0) = tri[0];
            tris(i,1) = tri[1];
            tris(i,2) = tri[2];
            i++;
        }
    }
    ObjFile.close();

}

void Mesh::set_geometry(){
}

void Mesh::set_rho() {
    rho.resize(npts,3);
    for(int i = 0; i < 20; i++){
        int ind = generateRandomInt(npts);
        rho.insert(ind,0) = generateRandomFloat();
        rho.insert(ind,1) = generateRandomFloat();
        rho.insert(ind,2) = generateRandomFloat();
    }
}
void Mesh::set_A()
{
    string line;
    ifstream ObjFile(filename);
    A.resize(npts,npts);
    while (getline(ObjFile, line)) {
        if(isFaceLine(line)){
            std::array<int,3> tri = objFileFaceLine(line);
            tri[0]--;
            tri[1]--;
            tri[2]--;
            if( A.coeff(tri[0],tri[1]) == 0 ){
                A.insert(tri[0],tri[1]) = 1;
                A.insert(tri[1],tri[0]) = 1;}
            if( A.coeff(tri[1],tri[2]) == 0 ){
                A.insert(tri[1],tri[2]) = 1;
                A.insert(tri[2],tri[1]) = 1;}
            if( A.coeff(tri[2],tri[0] ) == 0 ){
                A.insert(tri[0],tri[2]) = 1;
                A.insert(tri[2],tri[0]) = 1;}
        }
    }
    // Check Euler Characteristic
    // cout<<npts<<endl;
    // cout<<ntris<<endl;
    // cout<<A.sum()<<endl;
    // cout<<npts-A.sum()/2+ntris<<endl;
    nedges = A.sum()/2;
    ObjFile.close();
}

void Mesh::set_A0()
{
    string line;
    ifstream ObjFile(filename);
    A0.resize(npts,nedges);
    edges.resize(nedges,2);
    int e = 0 ;
    for(int p1 = 0; p1 < npts; p1++){
        for(int p2 = 0; p2 < p1; p2++){
            if(A.coeff(p1,p2) == 1){
                A0.insert(p1,e) = 1;
                edges(e,0) = p1;
                edges(e,1) = p2;
                e++;
            }
        }
    }
}

void Mesh::set_A1()
{
    // Lol worst possible implementation but probably fine because only runs once.
    string line;
    ifstream ObjFile(filename);
    A1.resize(nedges,ntris);
    for(int e = 0; e < nedges; e++){
        for(int t = 0; t < ntris; t++){
            int ep1 = edges(e,0);
            int ep2 = edges(e,1);
            int tp1 = tris(t,0);
            int tp2 = tris(t,1);
            int tp3 = tris(t,2);
            if(ep1 == tp1){
                if(ep2 == tp2){
                    A1.insert(e,t) = 1;
                }
                else if(ep2 == tp3){
                    A1.insert(e,t) = 1;
                }
            }
            else if(ep1 == tp2){
                if(ep2 == tp1){
                    A1.insert(e,t) = 1;
                }
                else if(ep2 == tp3){
                    A1.insert(e,t) = 1;
                }
            }
            else if(ep1 == tp3){
                if(ep2 == tp1){
                    A1.insert(e,t) = 1;
                }
                else if(ep2 == tp2){
                    A1.insert(e,t) = 1;
                }
            }
            
        }
    }
    A0A1 = A0*A1;
}

void Mesh::set_D_GL()
{
    GL.resize(npts,npts);
    D.resize(npts,npts);
    for( int i = 0; i<npts; i++){
        D.insert(i,i) = A.row(i).sum();
    }
    GL = D-A;

}

float Mesh::face_area(int face_idx)
{
    int v1_i = tris(face_idx,0);
    int v2_i = tris(face_idx,1);
    int v3_i = tris(face_idx,2);
    Eigen::Vector3d v1(pts(v1_i,Eigen::all));
    Eigen::Vector3d v2(pts(v2_i,Eigen::all));
    Eigen::Vector3d v3(pts(v3_i,Eigen::all));
    Eigen::Vector3d x1 = v1-v2;
    Eigen::Vector3d x2 = v1-v3;
    Eigen::Vector3d u = x1.cross(x2);
    cout << u.norm()/2 << endl;
    return u.norm()/2;
}

float Mesh::vertex_dual_area(int pt_idx)
{
    float area = 0.0;
    for(int t = 0; t < ntris; t++){
        if(A0A1.coeff(pt_idx,t) == 1){
            area += face_area(t);
        }
    }
    return area/3.0;
};

void Mesh::displayPts(int mode){
    glPointSize(5);
    glBegin(GL_POINTS);
    for(int i = 0; i < npts; i++){
        if(mode==0)
            glColor3f(1.0,1.0,1.0);
        else if(mode==1)
            glColor3f(rho.coeff(i,0),rho.coeff(i,1),rho.coeff(i,2));
        glVertex3d(pts(i,0), pts(i,1), pts(i,2));
    };
    glEnd();
}

void Mesh::displayTris(int mode){
    for(int i = 0; i < ntris; i++){
        int v1 = tris(i,0);
        int v2 = tris(i,1);
        int v3 = tris(i,2);
        glBegin(GL_POLYGON); 
        // glColor3f(generateRandomFloat(),generateRandomFloat(),generateRandomFloat());
        if(mode==0){
            glColor3f(0.7,0.7,0.7);
            glVertex3d(pts(v1-1,0), pts(v1-1,1), pts(v1-1,2));
            glVertex3d(pts(v2-1,0), pts(v2-1,1), pts(v2-1,2));
            glVertex3d(pts(v3-1,0), pts(v3-1,1), pts(v3-1,2));
        }
        if(mode==1){
            glColor3f(0.5,0.5,0.5);
            //glColor3f(rho(v1-1,0),rho(v1-1,1),rho(v1-1,2));
            glVertex3d(pts(v1-1,0), pts(v1-1,1), pts(v1-1,2));
            //glColor3f(rho(v2-1,0),rho(v2-1,1),rho(v2-1,2));
            glVertex3d(pts(v2-1,0), pts(v2-1,1), pts(v2-1,2));
            //glColor3f(rho(v3-1,0),rho(v3-1,1),rho(v3-1,2));
            glVertex3d(pts(v3-1,0), pts(v3-1,1), pts(v3-1,2));
        } 
        glEnd();
    };
    glEnd();
}

void Mesh::update_pts(){
    for(int i = 0; i < npts; i++){
        for(int j=0; j < 3; j++){
            pts(i,j) += (generateRandomFloat()-0.5)/50.0;
        }
    }
    last_update = std::clock();
}

void Mesh::reset_pts(){
    set_points();
    last_reset = std::clock();
}