
#include "../decs.h"

int spider_factor(int i) {

	if(i<0) i=0;
	//if(i>20) return 1;
	//int nj = pow(2,i+1);
	int nj = 2*pow(2,floor(log((i+0.5)*2)/log(2)));
	//fprintf(stderr,"%d %d\n", i, nj);
	nj = MIN(n2,nj);
	//if(i==1) nj /= 2;
	return n2/nj;

	//if(i<50) return 64;
	//if(i<100) return 32;
	//if(i<150) return 16;
	//if(i<200) return 8;
	//if(i<250) return 4;
	//if(i<300) return 2;
	//if(i<150) return 2;
	//if(i<250) return 4;
	//if(i<350) return 2;
	//else return 1;
}

void ijk_to_x(int i, int j, int k, double x[SPACEDIM]){

	x[0] = (i+0.5)*dx[0] + startx[0];
	#if(NDIM>1)
	if(j<istart[1]){ 
		x[1] = istart[1]*dx[1] + startx[1] + (j-istart[1]+0.5)*dx[1]*spider_fact[i];
	} else if(j>=istop[1]) {
		x[1] = istop[1]*dx[1] + startx[1] + (j-istop[1]+0.5)*dx[1]*spider_fact[i];
	} else {
		x[1] = j*dx[1] + startx[1] + 0.5*dx[1]*spider_fact[i];
	}
	#else
	x[1] = 0.5;
	#endif
	#if(NDIM==3)
	x[2] = (k+0.5)*dx[2] + startx[2];
	#else
	x[2] = 0.;
	#endif

	return;
}

void init_spider() {

	spider_fact = (int *)malloc_rank1(n1+2*NG, sizeof(int)) + NG;

	for(int i=-NG;i<n1+NG;i++) {
		spider_fact[i] = spider_factor(i);
		if(spider_fact[i] != 1) fprintf(stderr,"zone %d has spider_fact = %d\n", i, spider_fact[i]);
	}

	return;
}

double r_of_x(double x) {

	return rtrans*sinh(x/rtrans);
}

double x_of_r(double r) {

	double y = r/rtrans;

	return rtrans*log(y + sqrt(y*y + 1.));
}

double dr_dx(double x) {

	return cosh(x/rtrans);
}

double th_of_x(double x) {

	return acos(-x);
}

void x_to_rthphi(double x[SPACEDIM], double rvec[SPACEDIM]) {

	rvec[0] = r_of_x(x[0]);
	rvec[1] = acos(-x[1]);
	rvec[2] = x[2];
}	

void ijk_to_rthphi(int i, int j, int k, double rvec[SPACEDIM]) {

	double x[SPACEDIM];

	ijk_to_x(i, j, k, x);

	rvec[0] = r_of_x(x[0]);
	rvec[1] = acos(-x[1]);
	rvec[2] = x[2];

	return;
}

void ijk_to_r_dr(int i, int j, int k, double r[]) {

	double x[SPACEDIM];

	ijk_to_x(i, j, k, x);

	r[0] = r_of_x(x[0]-0.5*dx[0]);
	r[1] = r_of_x(x[0]);
	r[2] = r_of_x(x[0]+0.5*dx[0]);
}


double ijk_to_r(int i, int j, int k, double rhat[SPACEDIM]){

	double r,x[SPACEDIM];

	ijk_to_x(i, j, k, x);
	/*r = redge[i]+0.5*dr[i];
	r *= (1. + 2.*dr[i]*dr[i]/(dr[i]*dr[i] + 12.*r*r));
	x[0] = x_of_r(r);*/
	//r = rcenter[i];//r_of_x(x[0]);
	//x[0] = x_of_r(r);
	x[0] = beta0[i]/Gamma0[i];
	r = r_of_x(x[0]);
	rhat[0] = dr_dx(x[0]);
	rhat[1] = 0.;
	rhat[2] = 0.;

	return r;
}


// ************************* //
// THIS NEEDS TO BE FIXED!!! //
// ************************* //
void vec_transform_to_xyz(float *vp, int i, int j, int k) {

	int l,m;
	double x[SPACEDIM];
	float v[SPACEDIM];
	double lam[SPACEDIM][SPACEDIM];
	double r,sth,cth,sph,cph;

	ijk_to_x(i, j, k, x);

	r = r_of_x(x[0]);
	sth = sin(x[1]);
	cth = cos(x[1]);
	sph = sin(x[2]);
	cph = cos(x[2]);

	// x
	lam[0][0] = sth*cph;	// r
	lam[0][1] = cth*cph;	// theta
	lam[0][2] = -sth*sph;	// phi

	// y
	lam[1][0] = sth*sph;
	lam[1][1] = cth*sph;
	lam[1][2] = sth*cph;


	// z
	lam[2][0] = cth;
	lam[2][1] = -sth;
	lam[2][2] = 0.;

	v[0] = v[1] = v[2] = 0.;

	for(l=0;l<SPACEDIM;l++) {
		for(m=0;m<SPACEDIM;m++) {
			v[l] += lam[l][m]*vp[m]*ND_ELEM(geom,i,j,k).scale[0][m];
		}
	}

	#if(NDIM==2)
	vp[0] = v[0];
	vp[1] = v[2];
	vp[2] = v[1];
	#else
	vp[0] = v[0];
	vp[1] = v[1];
	vp[2] = v[2];
	#endif

	return;
}

void init_coords() {

	int ii;

	for(ii=istart[0]-NG;ii<istop[0]+NG;ii++) {
		redge[ii] = r_of_x(fabs(ii*dx[0]+startx[0]));
		//rcenter[ii] = fabs(r_of_x((ii+0.5)*dx[0]+startx[0]));
		dr[ii] = r_of_x(fabs((ii+1)*dx[0]+startx[0])) - redge[ii];
		rcenter[ii] = redge[ii]+0.5*dr[ii];//0.5*(redge[ii] + 0.5*dr[ii] + r_of_x((ii+0.5)*dx[0]+startx[0]));
		//fprintf(stderr,"%d %g\n", ii, redge[ii]);
		//spider_fact[ii] = spider_factor(ii);
		//if(spider_fact[ii] != 1) fprintf(stderr,"zone %d has spider_fact = %d\n", ii, spider_fact[ii]);
	}

}


// FIX
void init_interp_vol() {

	double x0,x1,r0,r1;

	fprintf(stderr,"initializing alpha,beta,Gamma...\n");
	for(int i=istart[0]-NG;i<istop[0]+NG;i++) {
		x0 = startx[0] + i*dx[0];
		x1 = x0 + dx[0];
		r0 = r_of_x(x0);
		r1 = r_of_x(x1);
		Gamma0[i] = pow(rtrans,3.)*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.))/3.;
		beta0[i] = pow(rtrans,3.)*(rtrans*(cosh(3.*x0/rtrans) + 9.*cosh(x1/rtrans) - 9.*cosh(x0/rtrans) - cosh(3.*x1/rtrans)) - 12.*x0*pow(sinh(x0/rtrans),3.) + 12.*x1*pow(sinh(x1/rtrans),3.))/36.;
		alpha0[i] = pow(rtrans,3.)*(6.*rtrans*x0*cosh(3.*x0/rtrans) - 54.*rtrans*x0*cosh(x0/rtrans) + 54.*rtrans*x1*cosh(x1/rtrans) - 6.*rtrans*x1*cosh(3.*x1/rtrans) 
				+ 27.*(2.*rtrans*rtrans + x0*x0)*sinh(x0/rtrans) - (2.*rtrans*rtrans + 9.*x0*x0)*sinh(3.*x0/rtrans) - 27.*(2.*rtrans*rtrans + x1*x1)*sinh(x1/rtrans) + (2.*rtrans*rtrans + 9.*x1*x1)*sinh(3.*x1/rtrans))/108.;

		nu0[i] = pow(rtrans,3.)*(rtrans*(2.*rtrans*rtrans + 9.*x0*x0)*cosh(3.*x0/rtrans) - 81.*rtrans*(2.*rtrans*rtrans + x0*x0)*cosh(x0/rtrans) + 81.*rtrans*(2.*rtrans*rtrans + x1*x1)*cosh(x1/rtrans) - rtrans*(2.*rtrans*rtrans + 9.*x1*x1)*cosh(3.*x1/rtrans) - 3.*x0*((2.*rtrans*rtrans + 3.*x0*x0)*sinh(3.*x0/rtrans) - 9.*(6.*rtrans*rtrans + x0*x0)*sinh(x0/rtrans)) + 3.*x1*((2.*rtrans*rtrans + 3.*x1*x1)*sinh(3.*x1/rtrans) - 9.*(6.*rtrans*rtrans + x1*x1)*sinh(x1/rtrans)))/108.;

		mu0[i] = pow(rtrans,3.)*(12.*rtrans*x0*(2.*rtrans*rtrans + 3.*x0*x0)*cosh(3.*x0/rtrans) - 324.*rtrans*x0*(6.*rtrans*rtrans + x0*x0)*cosh(x0/rtrans) + 324.*rtrans*x1*(6.*rtrans*rtrans + x1*x1)*cosh(x1/rtrans) - 12.*rtrans*x1*(2.*rtrans*rtrans +3.*x1*x1)*cosh(3.*x1/rtrans) + 81.*(24.*pow(rtrans,4.) + 12.*rtrans*rtrans*x0*x0 + pow(x0,4.))*sinh(x0/rtrans) - (8.*pow(rtrans,4.) + 36.*rtrans*rtrans*x0*x0 + 27*pow(x0,4.))*sinh(3*x0/rtrans) - 81.*(24.*pow(rtrans,4.) + 12.*rtrans*rtrans*x1*x1 +pow(x1,4.))*sinh(x1/rtrans) + (8.*pow(rtrans,4.) + 36.*rtrans*rtrans*x1*x1 + 27*pow(x1,4.))*sinh(3.*x1/rtrans))/324.;

		/*Gamma0[i] = (pow(r1,3.)-pow(r0,3.))/3.;
		beta0[i] = (pow(r1,4.)-pow(r0,4.))/4.;
		alpha0[i] = (pow(r1,5.)-pow(r0,5.))/5.;
		nu0[i] = (pow(r1,6.)-pow(r0,6.))/6.;
		mu0[i] = (pow(r1,7.)-pow(r0,7.))/7.;*/


	}

	#if(NDIM>1)
	for(int j=istart[1]-NG; j<istop[1]+NG; j++) {
		x0 = startx[1] + j*dx[1];
		x1 = x0 + dx[1];
		Gamma1[j] = dx[1];
		beta1[j] = 0.5*(x1*x1 - x0*x0);
		alpha1[j] = (pow(x1,3.) - pow(x0,3.))/3.;
		nu1[j] = (pow(x1,4.) - pow(x0,4.))/4.;
		mu1[j] = (pow(x1,5.) - pow(x0,5.))/5.;
	}
	#endif

	#if(NDIM==3)
	for(int k=istart[2]-NG; k<istop[2]+NG; k++) {
		x0 = startx[2] + k*dx[2];
		x1 = x0 + dx[2];
		Gamma2[k] = dx[2];
		beta2[k] = 0.5*(x1*x1 - x0*x0);
		alpha2[k] = (pow(x1,3.) - pow(x0,3.))/3.;
		nu2[k] = (pow(x1,4.) - pow(x0,4.))/4.;
		mu2[k] = (pow(x1,5.) - pow(x0,5.))/5.;
	}
	#endif

	fprintf(stderr,"done!\n");

	return;
}

// FIX
void calc_interp_coeffs1(int i, double *mu, double *nu, double *alpha, double *beta, double *Gamma, double *x) {

	if(spider_fact[i] == 1) {
		for(int j=istart[1]-NG; j<istop[1]+NG;j++) {
			mu[j-istart[1]] = mu1[j];
			nu[j-istart[1]] = nu1[j];
			alpha[j-istart[1]] = alpha1[j];
			beta[j-istart[1]] = beta1[j];
			Gamma[j-istart[1]] = Gamma1[j];
			x[j-istart[1]] = startx[1] + j*dx[1];
		}
	} else {
		for(int j=istart[1]-NG; j<istart[1]; j++) {
			double x0 = startx[1] + spider_fact[i]*j*dx[1];
			double x1 = x0 + spider_fact[i]*dx[1];
			Gamma[j-istart[1]] = x1-x0;
			beta[j-istart[1]] = 0.5*(x1*x1 - x0*x0);
			alpha[j-istart[1]] = (pow(x1,3.) - pow(x0,3.))/3.;
			nu[j-istart[1]] = (pow(x1,4.) - pow(x0,4.))/4.;
			mu[j-istart[1]] = (pow(x1,5.) - pow(x0,5.))/5.;
			x[j-istart[1]] = x0;
		}
		for(int j=istart[1]; j<istop[1]; j+=spider_fact[i]) {
			int ind = (j-istart[1])/spider_fact[i];
			mu[ind] = mu1[j];
			nu[ind] = nu1[j];
			alpha[ind] = alpha1[j];
			beta[ind] = beta1[j];
			Gamma[ind] = Gamma1[j];
			x[ind] = startx[1] + j*dx[1];
			for(int s=1; s<spider_fact[i]; s++) {
				mu[ind] += mu1[j+s];
				nu[ind] += nu1[j+s];
				alpha[ind] += alpha1[j+s];
				beta[ind] += beta1[j+s];
				Gamma[ind] += Gamma1[j+s];
			}
		}
		for(int j=istop[1]; j<istop[1]+NG; j++) {
			double x0 = startx[1] + istop[1]*dx[1] + spider_fact[i]*(j-istop[1])*dx[1];
			double x1 = x0 + spider_fact[i]*dx[1];
			int ind = (istop[1]-istart[1])/spider_fact[i] + j-istop[1];
			Gamma[ind] = x1 - x0;
			beta[ind] = 0.5*(x1*x1 - x0*x0);
			alpha[ind] = (pow(x1,3.) - pow(x0,3.))/3.;
			nu[ind] = (pow(x1,4.) - pow(x0,4.))/4.;
			mu[ind] = (pow(x1,5.) - pow(x0,5.))/5.;
			x[ind] = x0;
		}
	}
	return;
}

void calc_interp_coeffs2(int i, double *mu, double *nu, double *alpha, double *beta, double *Gamma, double *x) {

	if(spider_fact[i] == 1) {
		for(int k=istart[2]-NG; k<istop[2]+NG; k++) {
			mu[k-istart[2]] = mu2[k];
			nu[k-istart[2]] = nu2[k];
			alpha[k-istart[2]] = alpha2[k];
			beta[k-istart[2]] = beta2[k];
			Gamma[k-istart[2]] = Gamma2[k];
			x[k-istart[2]] = startx[2] + k*dx[2];
		}
	} else {
		for(int k=istart[2]-NG; k<istart[2]; k++) {
			double x0 = startx[2] + k*dx[2];
			double x1 = x0 + dx[2];
			Gamma[k-istart[2]] = dx[2];
			beta[k-istart[2]] = 0.5*(x1*x1 - x0*x0);
			alpha[k-istart[2]] = (pow(x1,3.) - pow(x0,3.))/3.;
			nu[k-istart[2]] = (pow(x1,4.) - pow(x0,4.))/4.;
			mu[k-istart[2]] = (pow(x1,5.) - pow(x0,5.))/5.;
			x[k-istart[2]] = x0;
		}
		for(int k=istart[2]; k<istop[2]; k+=spider_fact[i]) {
			int ind = (k-istart[2])/spider_fact[i];
			mu[ind] = mu2[k];
			nu[ind] = nu2[k];
			alpha[ind] = alpha2[k];
			beta[ind] = beta2[k];
			Gamma[ind] = Gamma2[k];
			x[ind] = startx[2] + spider_fact[i]*k*dx[2];
			for(int s=1; s<spider_fact[i]; s++) {
				mu[ind] += mu2[k+s];
				nu[ind] += nu2[k+s];
				alpha[ind] += alpha2[k+s];
				beta[ind] += beta2[k+s];
				Gamma[ind] += Gamma2[k+s];
			}
		}
		for(int k=istop[2]; k<istop[2]+NG; k++) {
			double x0 = startx[2] + istop[2]*dx[2] + spider_fact[i]*(k-istop[2])*dx[2];
			double x1 = x0 + spider_fact[i]*dx[2];
			int ind = (istop[2]-istart[2])/spider_fact[i] + k-istop[2];
			Gamma[ind] = dx[2];
			beta[ind] = 0.5*(x1*x1 - x0*x0);
			alpha[ind] = (pow(x1,3.) - pow(x0,3.))/3.;
			nu[ind] = (pow(x1,4.) - pow(x0,4.))/4.;
			mu[ind] = (pow(x1,5.) - pow(x0,5.))/5.;
			x[ind] = x0;

		}
	}
	return;
}
		

void init_volume() {

	int ii,jj,kk;
	double th0,th1;
	double d2x,spower;

	#if(NDIM==1)
	d2x = 4.*M_PI;
	#elif(NDIM==2)
	d2x = 2.*M_PI*dx[1];
	spower = 1;
	#else
	d2x = dx[1]*dx[2];
	spower = 2;
	#endif

	for(ii=istart[0]-NG;ii<istop[0]+NG;ii++) {
		double V = d2x*(pow(redge[ii]+dr[ii],3.) - pow(redge[ii],3.))/3.;
		#if(NDIM==1)
		geom[ii].volume = V;
		#endif
		#if(NDIM>1)
		for(jj=istart[1]-NG;jj<istop[1]+NG;jj++) {
		#endif
		#if(NDIM==3)
		for(kk=istart[2]-NG;kk<istop[2]+NG;kk++) {
		#endif
			ND_ELEM(geom,ii,jj,kk).volume = V*pow(spider_fact[ii],spower);
		#if(NDIM==3)
		}
		#endif
		#if(NDIM>1)
		}
		#endif
	}

	return;

}

void init_area() {

	int ii,jj,kk,loc;
	double x[SPACEDIM];
	double r,drdx;

	double d2x,spower;

	#if(NDIM==1)
	d2x = 4.*M_PI;
	#elif(NDIM==2)
	d2x = 2.*M_PI*dx[1];
	spower = 1;
	#else
	d2x = dx[1]*dx[2];
	spower = 2;
	#endif
	
	for(ii=istart[0];ii<=istop[0];ii++) {
	#if(NDIM>1)
	for(jj=istart[1];jj<=istop[1];jj++) {
	#endif
	#if(NDIM==3)
	for(kk=istart[2];kk<=istop[2];kk++) {
	#endif
		ND_ELEM(geom,ii,jj,kk).area[0] = d2x*pow(spider_fact[ii],spower)*redge[ii]*redge[ii]*cosh((ii*dx[0]+startx[0])/rtrans);

		#if(NDIM>1)
		ND_ELEM(geom,ii,jj,kk).area[1] = ND_ELEM(geom,ii,jj,kk).volume/(spider_fact[ii]*dx[1]);
		//if(jj==0 || jj == n2) ND_ELEM(geom,ii,jj,kk).area[1] = 0.;
		#endif

		#if(NDIM==3)
		ND_ELEM(geom,ii,jj,kk).area[2] = ND_ELEM(geom,ii,jj,kk).volume/(spider_fact[ii]*dx[2]);
		#endif

	#if(NDIM==3)
	}
	#endif
	#if(NDIM>1)
	}
	#endif
	}

/*
	for(ii=istart[0];ii<=istop[0];ii++) {
		double a = 0;
		for(jj=istart[1];jj<istop[1];jj+=spider_fact[ii]) {
			a += ND_ELEM(geom,ii,jj,0).area[0];
		}
		fprintf(stdout,"%d %g\n", ii, a/(4.*M_PI*redge[ii]*redge[ii]*cosh((ii*dx[0]+startx[0])/rtrans) + 1.e-30));
	}
*/

	return;
}
		

void init_conn() {

	int ii,jj,kk;
	double xr0,xr1,xt0,xt1,xtm;

	for(ii=istart[0];ii<istop[0];ii++) {
		xr0 = ii*dx[0] + startx[0];
		xr1 = (ii+1)*dx[0] + startx[0];
		#if(NDIM>1)
		for(jj=istart[1];jj<istop[1];jj+=spider_fact[ii]) {
		xt0 = jj*dx[1] + startx[1];
		xt1 = (jj+spider_fact[ii])*dx[1] + startx[1];
		xtm = 0.5*(xt0+xt1);
		#endif
		#if(NDIM==3)
		for(kk=istart[2];kk<istop[2];kk+=spider_fact[ii]) {
		#endif

		memset(&ND_ELEM(geom,ii,jj,kk).conn[0][0][0], 0, 27*sizeof(double));

		ND_ELEM(geom,ii,jj,kk).conn[0][0][0] = (-9.*cosh(xr0/rtrans) + cosh(3.*xr0/rtrans) + 9.*cosh(xr1/rtrans) - cosh(3.*xr1/rtrans))/(4.*rtrans*(pow(sinh(xr0/rtrans),3.)-pow(sinh(xr1/rtrans),3.)));

		// set this below
		//ND_ELEM(geom,ii,jj,kk).conn[0][1][1]

		// set this below
		//ND_ELEM(geom,ii,jj,kk).conn[0][2][2] = -ND_ELEM(geom,ii,jj,kk).conn[0][0][0]/(3.*dx[1]) * (pow(xt0,3.) - 3.*xt0 + 3.*xt1 - pow(xt1,3.));

		ND_ELEM(geom,ii,jj,kk).conn[1][0][1] = (pow(cosh(xr1/rtrans),3.) - pow(cosh(xr0/rtrans),3.))/(rtrans*(pow(sinh(xr1/rtrans),3.) - pow(sinh(xr0/rtrans),3.)));
		ND_ELEM(geom,ii,jj,kk).conn[1][1][0] = ND_ELEM(geom,ii,jj,kk).conn[1][0][1];

		ND_ELEM(geom,ii,jj,kk).conn[1][1][1] = xtm/(1. - xtm*xtm);

		// set this below
		//ND_ELEM(geom,ii,jj,kk).conn[1][2][2]

		ND_ELEM(geom,ii,jj,kk).conn[2][0][2] = ND_ELEM(geom,ii,jj,kk).conn[1][0][1];
		ND_ELEM(geom,ii,jj,kk).conn[2][2][0] = ND_ELEM(geom,ii,jj,kk).conn[1][0][1];
		ND_ELEM(geom,ii,jj,kk).conn[2][1][2] = -ND_ELEM(geom,ii,jj,kk).conn[1][1][1];
		ND_ELEM(geom,ii,jj,kk).conn[2][2][1] = -ND_ELEM(geom,ii,jj,kk).conn[1][1][1];


		ND_ELEM(geom,ii,jj,kk).conn[0][1][1] = -ND_ELEM(geom,ii,jj,kk).conn[1][1][0] * ND_ELEM(geom,ii,jj,kk).gcov[0][1][1]/ND_ELEM(geom,ii,jj,kk).gcov[0][0][0];
		ND_ELEM(geom,ii,jj,kk).conn[0][2][2] = -ND_ELEM(geom,ii,jj,kk).conn[2][2][0] * ND_ELEM(geom,ii,jj,kk).gcov[0][2][2]/ND_ELEM(geom,ii,jj,kk).gcov[0][0][0];
		ND_ELEM(geom,ii,jj,kk).conn[1][2][2] = -ND_ELEM(geom,ii,jj,kk).conn[2][2][1] * ND_ELEM(geom,ii,jj,kk).gcov[0][2][2]/ND_ELEM(geom,ii,jj,kk).gcov[0][1][1];

		#if(NDIM==3)
		}
		#endif
		#if(NDIM>1)
		}
		#endif
	}



	//ND_ELEM(geom,0,spider_fact[0],0).conn[2][1][2] = -ND_ELEM(geom,0,0,0).conn[2][1][2];

	return;
}

void init_gcov() {

	double x[SPACEDIM],r[2],drdx[2];
	int loc,i,j,ii,jj,kk;
	double th0,th1;

	for(int i=istart[0]-NG; i<istop[0]+NG; i++) {
	#if(NDIM>1)
	for(int j=istart[1]-NG; j<istop[1]+NG; j++) {
	#endif
	#if(NDIM==3)
	for(int k=istart[2]-NG; k<istop[2]+NG; k++) {
	#endif
		double x0 = startx[0] + i*dx[0];
		double x1 = x0 + dx[0];
		if(j<istart[1]) {
			th0 = startx[1] + j*spider_fact[i]*dx[1];
			th1 = th0 + spider_fact[i]*dx[1];
		} else if(j>=istop[1]) {
			th0 = startx[1] + istop[1]*dx[1] + (j-istop[1])*spider_fact[i]*dx[1];
			th1 = th0 + spider_fact[i]*dx[1];
		} else {
			th0 = startx[1] + j*dx[1];
			th1 = th0 + spider_fact[i]*dx[1];
		}

		//double th0 = startx[1] + j*dx[1];
		//double th1 = th0 + spider_fact[i]*dx[1];

		// initialize to zero
		for(loc=0;loc<=NDIM;loc++) {
			for(int m=0;m<SPACEDIM;m++) {
				for(int l=0;l<SPACEDIM;l++) {
					ND_ELEM(geom,i,j,k).gcov[loc][m][l] = 0.;
				}
			}
		}


		// volume average
		ND_ELEM(geom,i,j,k).gcov[0][0][0] = ((7. + 3.*cosh(2.*x1/rtrans))*pow(sinh(x1/rtrans),3.) - (7.+3.*cosh(2.*x0/rtrans))*pow(sinh(x0/rtrans),3.))/(10.*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.)));
		ND_ELEM(geom,i,j,k).gcov[0][1][1] = 3.*rtrans*rtrans*(pow(sinh(x1/rtrans),5.) - pow(sinh(x0/rtrans),5.))/(5.*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.))) * 1./(1 - pow(0.5*(th0+th1),2.));
		ND_ELEM(geom,i,j,k).gcov[0][2][2] = 3.*rtrans*rtrans*(pow(sinh(x1/rtrans),5) - pow(sinh(x0/rtrans),5))/(5.*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.))) * (pow(th0,3.) - 3.*th0 + 3*th1 - pow(th1,3.))/(3.*dx[1]*spider_fact[i]);

		// 0-direction -- face-averaged
		ND_ELEM(geom,i,j,k).gcov[1][0][0] = pow(dr_dx(x0),2.);
		ND_ELEM(geom,i,j,k).gcov[1][1][1] = pow(r_of_x(x0),2.) * 1./(1. - pow(0.5*(th0+th1),2.));
		ND_ELEM(geom,i,j,k).gcov[1][2][2] = pow(r_of_x(x0),2)*(pow(th0,3.) - 3*th0 + 3*th1 - pow(th1,3))/(3*dx[1]*spider_fact[i]);

		#if(NDIM>1)
		// 1-direction -- face-averaged
		ND_ELEM(geom,i,j,k).gcov[2][0][0] = ND_ELEM(geom,i,j,k).gcov[0][0][0];
		ND_ELEM(geom,i,j,k).gcov[2][1][1] = 3.*rtrans*rtrans*(pow(sinh(x1/rtrans),5) - pow(sinh(x0/rtrans),5))/(5.*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.))) * 1./(1. - pow(th0,2.));
		if(j==0 || j==n2) ND_ELEM(geom,i,j,k).gcov[2][1][1] = 3.*rtrans*rtrans*(pow(sinh(x1/rtrans),5) - pow(sinh(x0/rtrans),5))/(5.*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.))) * 1./(1. - pow(th1/100.,2.));;

		ND_ELEM(geom,i,j,k).gcov[2][2][2] = 3.*rtrans*rtrans*(pow(sinh(x1/rtrans),5) - pow(sinh(x0/rtrans),5))/(5.*(pow(sinh(x1/rtrans),3.) - pow(sinh(x0/rtrans),3.))) * (1. - th0*th0);
		#endif

		#if(NDIM==3)
		// 2-direction -- face-averaged
		ND_ELEM(geom,i,j,k).gcov[3][0][0] = ND_ELEM(geom,i,j,k).gcov[0][0][0];
		ND_ELEM(geom,i,j,k).gcov[3][1][1] = ND_ELEM(geom,i,j,k).gcov[0][1][1];
		ND_ELEM(geom,i,j,k).gcov[3][2][2] = ND_ELEM(geom,i,j,k).gcov[0][2][2];
		#endif
	#if(NDIM==3)
	}
	#endif
	#if(NDIM>1)
	}
	#endif
	}

	// fill in dead zones to get scale factors right for output
	#if(NDIM>1)
	for(int i=istart[0]; i<istop[0]; i++) {
	for(int j=istart[1]; j<istop[1]; j++) {
	#if(NDIM==3)
	for(int k=istart[2]; k<istop[2]; k++) {
	#endif
		if(spider_fact[i] == 1) break;
		memcpy(ND_ELEM(geom,i,j,k).gcov, ND_ELEM(geom,i,j-j%spider_fact[i],k-k%spider_fact[i]).gcov, (1+NDIM)*SPACEDIM*SPACEDIM*sizeof(double));
	#if(NDIM==3)
	}
	#endif
	}
	}
	#endif

	return;
}

void init_gcon() {

	int ii,jj,kk,loc,i,j;

	for(ii=istart[0]-NG;ii<istop[0]+NG;ii++) {
	#if(NDIM>1)
	for(jj=istart[1]-NG;jj<istop[1]+NG;jj++) {
	#endif
	#if(NDIM==3)
	for(kk=istart[2]-NG;kk<istop[2]+NG;kk++) {
	#endif
	//	for(loc=0;loc<1+NDIM;loc++) {
		for(i=0;i<SPACEDIM;i++) {
			for(j=0;j<SPACEDIM;j++) {
				if(fabs(ND_ELEM(geom,ii,jj,kk).gcov[0][i][j]) > 1.e-50) {
					ND_ELEM(geom,ii,jj,kk).gcon[i][j] = 1./ND_ELEM(geom,ii,jj,kk).gcov[0][i][j];
				} else {
					ND_ELEM(geom,ii,jj,kk).gcon[i][j] = 0.;
				}
			}
		}
	//	}
	#if(NDIM==3)
	}
	#endif
	#if(NDIM>1)
	}
	#endif
	}

	return;
}

void init_scale() {

	int ii,jj,kk,dd,loc;

	for(ii=istart[0]-NG;ii<istop[0]+NG;ii++) {
	#if(NDIM>1)
	for(jj=istart[1]-NG;jj<istop[1]+NG;jj++) {
	#endif
	#if(NDIM==3)
	for(kk=istart[2]-NG;kk<istop[2]+NG;kk++) {
	#endif
		for(loc=0;loc<=NDIM;loc++) {
			SLOOP {
				ND_ELEM(geom,ii,jj,kk).scale[loc][dd] = sqrt(ND_ELEM(geom,ii,jj,kk).gcov[loc][dd][dd]);
			}
		}
	#if(NDIM==3)
	}
	#endif
	#if(NDIM>1)
	}
	#endif
	}


	return;
}
