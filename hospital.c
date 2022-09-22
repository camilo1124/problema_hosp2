/*Inclusion de librerias necesarias para la simulacios*/

#include <stdio.h>  
#include <stdlib.h>
#include <math.h>
#include "lcgrand.h"  /*Libreria para generar numeros aleatorios*/

#define OCUPADO		1
#define LIBRE		0

double media_servicio,prob;
int muestra,pacientes_acum,sin_silla,k_sillas,tasa_poisson,estado_sistema,lim_horas;
float tiempo_acum,probabilidad_resp;

FILE *infile,*outfile;

void inicializar();
void llegadas();
void salidas(int grupo,float servicios,float servicios_hora);
void reporte();

double* f_poisson();
double* F_poisson();
int poisson_inv(float r);
float expon(float media);

int main(){

	infile  = fopen("hospital.in","r");
	outfile = fopen("hospital.out","w");
	k_sillas = 0;

	fscanf(infile, "%lf %d %d", &media_servicio, &tasa_poisson,&lim_horas);

	fprintf(outfile, "Simulacion del comportamiento del hospital\n\n");
	fprintf(outfile, "Tipo de cola fifo M/M/1\n\n");
	fprintf(outfile, "Promedio de pacientes por hora (poisson): %d\n",tasa_poisson);
	fprintf(outfile, "Tiempo medio de servicio o atencion medica (exp): %.1lf\n", media_servicio);
	fprintf(outfile, "Horas de cada simulacion de comportamiento hospital: %d",lim_horas);

	probabilidad_resp = 1;

	while (probabilidad_resp > 0.1){
		

		inicializar();
		for (int hora = 1; hora <= lim_horas; hora++){
			llegadas();

		}
		k_sillas++;
		probabilidad_resp = (float)sin_silla/(float)muestra;
		
	}
	k_sillas--;

	reporte();

	fclose(infile);
	fclose(outfile);
	return 0;
}

void reporte(){
	fprintf(outfile,"\nObjetivo: Determinar el numero de sillas minimo para que la probabilidad\n");
	fprintf(outfile,"de que un paciente llegue y espere de pie sea inferior al 0.10");
	fprintf(outfile, "\n\nMuestra recogida - Pacientes en la sala de espera : %d\n",muestra);
	fprintf(outfile, "Numero de sillas minimo: %d\n", k_sillas);
	fprintf(outfile, "Probabilidad obtenida: %f\n",probabilidad_resp);
}

void inicializar(){
	
	/*Inicializa las variables del sistema */
	
	estado_sistema = LIBRE;
	muestra = 0;
	pacientes_acum = 0;
	sin_silla = 0;
	tiempo_acum = 0;

}

void llegadas(){
	double hora = 60;
	int grupo_poisson = poisson_inv(lcgrand(1));
	int grupo = grupo_poisson + pacientes_acum;
	float servicios = 0,servicios_hora = 0 + tiempo_acum;
	if (estado_sistema == LIBRE){
		muestra += grupo_poisson-1;
		if ((grupo-1) > k_sillas){
			sin_silla += (grupo-1) - k_sillas;
		}
		salidas(grupo,servicios,servicios_hora);
	}else{
		muestra += grupo_poisson;
		if (grupo > k_sillas){
			if (pacientes_acum < k_sillas){
				sin_silla += grupo_poisson - (k_sillas - pacientes_acum);
			}else{
				sin_silla += grupo_poisson;
			}
		}
		salidas(grupo,servicios,servicios_hora);
	} 
}

void salidas(int grupo,float servicios,float servicios_hora){
	for (int i = 1; i <= grupo; i++){
		servicios = expon(media_servicio);
		if ((servicios_hora + servicios) > 60){
			tiempo_acum = (servicios_hora + servicios) - 60;
			servicios_hora = 60.0;
			pacientes_acum = grupo - i;
			estado_sistema = OCUPADO;
			break;
		}else{
			servicios_hora += servicios;
			if (i == grupo){
				tiempo_acum = 0;
				pacientes_acum = 0;
				estado_sistema = LIBRE;
			}
		}
	}
}

double* f_poisson(){
	double* f_x;
	f_x = (double*)malloc(21*sizeof(double));
	f_x[0] = 0;
	for (int x = 1; x <= 20; x++){
		double x_fac = 1;
		for (int fac = 1; fac <= x; fac++){
			x_fac = x_fac * fac;
		}
		f_x[x] = (pow(exp(1),-tasa_poisson) * pow(tasa_poisson,x)) / x_fac;
	}
	return (f_x);
}

double* F_poisson(){
	double* f_x = f_poisson();
	double* F_x;
	F_x = (double*)malloc(21*sizeof(double));

	for (int i = 0; i < 21; i++){
		double suma = 0;
		for (int j = 0; j <= i; j++){
			suma += f_x[j];
		}
		if (suma > 1){
			F_x[i] = 1;
		}else{
			F_x[i] = suma;
		}
	}
	return (F_x);
}

int poisson_inv(float r){
	double* FF_x = F_poisson();
	for (int i = 0; i < 21; i++){
		if (FF_x[i] >= r){
			free(FF_x);			
			return i;
		}
	}
	return 21;
}

float expon(float media){
	return -media * log(lcgrand(3));
}
