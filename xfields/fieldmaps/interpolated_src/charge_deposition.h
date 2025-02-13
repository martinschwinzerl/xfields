#ifndef XFIELDS_CHARGE_DEPOSITION_H
#define XFIELDS_CHARGE_DEPOSITION_H

//include_file atomicadd.clh for_context opencl
//include_file atomicadd.h for_context cpu_serial cpu_openmp

/*gpukern*/ void p2m_rectmesh3d(
        // INPUTS:
          // length of x, y, z arrays
        const int nparticles,
          // particle positions
        /*gpuglmem*/ const double* x, 
	/*gpuglmem*/ const double* y, 
	/*gpuglmem*/ const double* z,
	  // particle weights
	/*gpuglmem*/ const double* part_weights,
          // mesh origin
        const double x0, const double y0, const double z0,
          // mesh distances per cell
        const double dx, const double dy, const double dz,
          // mesh dimension (number of cells)
        const int nx, const int ny, const int nz,
        // OUTPUTS:
        /*gpuglmem*/ double *grid1d
) {

    double vol_m1 = 1/(dx*dy*dz);
    
    #pragma omp parallel for //only_for_context cpu_openmp 
    for (int pidx=0; pidx<nparticles; pidx++){ //vectorize_over pidx nparticles

    	double pwei = part_weights[pidx];

    	// indices
    	int jx = floor((x[pidx] - x0) / dx);
    	int ix = floor((y[pidx] - y0) / dy);
    	int kx = floor((z[pidx] - z0) / dz);

    	// distances
    	double dxi = x[pidx] - (x0 + jx * dx);
    	double dyi = y[pidx] - (y0 + ix * dy);
    	double dzi = z[pidx] - (z0 + kx * dz);

    	// weights
    	double wijk =    pwei * vol_m1 * (1.-dxi/dx) * (1.-dyi/dy) * (1.-dzi/dz);
    	double wi1jk =   pwei * vol_m1 * (1.-dxi/dx) * (dyi/dy)    * (1.-dzi/dz);
    	double wij1k =   pwei * vol_m1 * (dxi/dx)    * (1.-dyi/dy) * (1.-dzi/dz);
    	double wi1j1k =  pwei * vol_m1 * (dxi/dx)    * (dyi/dy)    * (1.-dzi/dz);
    	double wijk1 =   pwei * vol_m1 * (1.-dxi/dx) * (1.-dyi/dy) * (dzi/dz);
    	double wi1jk1 =  pwei * vol_m1 * (1.-dxi/dx) * (dyi/dy)    * (dzi/dz);
    	double wij1k1 =  pwei * vol_m1 * (dxi/dx)    * (1.-dyi/dy) * (dzi/dz);
    	double wi1j1k1 = pwei * vol_m1 * (dxi/dx)    * (dyi/dy)    * (dzi/dz);

    	if (jx >= 0 && jx < nx - 1 && ix >= 0 && ix < ny - 1
	    	    && kx >= 0 && kx < nz - 1)
    	{
    	    atomicAdd(&grid1d[jx   + ix*nx     + kx*nx*ny],     wijk);
    	    atomicAdd(&grid1d[jx+1 + ix*nx     + kx*nx*ny],     wij1k);
    	    atomicAdd(&grid1d[jx   + (ix+1)*nx + kx*nx*ny],     wi1jk);
    	    atomicAdd(&grid1d[jx+1 + (ix+1)*nx + kx*nx*ny],     wi1j1k);
    	    atomicAdd(&grid1d[jx   + ix*nx     + (kx+1)*nx*ny], wijk1);
    	    atomicAdd(&grid1d[jx+1 + ix*nx     + (kx+1)*nx*ny], wij1k1);
    	    atomicAdd(&grid1d[jx   + (ix+1)*nx + (kx+1)*nx*ny], wi1jk1);
    	    atomicAdd(&grid1d[jx+1 + (ix+1)*nx + (kx+1)*nx*ny], wi1j1k1);
    	}
    }//end_vectorize
}

#endif
