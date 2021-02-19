#pragma once

#include "headers.h"

class pointeur_dimage
{
public:
	int size;
	vx_image* p_vx_image;

	pointeur_dimage() : p_vx_image(0), size(0) {}

	~pointeur_dimage()
	{
		for (int i = 0; i < size; i++)
		{
			vxReleaseImage(&p_vx_image[i]);
		}
		delete(p_vx_image); p_vx_image = 0;
	}

	int add_image(vx_image img)
	{
		pointeur_dimage p;
		p.p_vx_image = new vx_image[size + 1];
		for (int i = 0; i < size; i++) p.p_vx_image[i] = p_vx_image[i];
		p.p_vx_image[size] = img;
		p_vx_image = p.p_vx_image;
		size++;
	}

	vx_image operator[] (int i)
	{
		return p_vx_image[i];
	}

	bool delete_image(int i)
	{
		if(!(VX_SUCCESS == vxReleaseImage(&p_vx_image[i]))) return false;
		pointeur_dimage p;
		p.p_vx_image = new vx_image[size - 1];
		for (int k = 0; k < i; k++) p.p_vx_image[k] = p_vx_image[k];
		for (int k = i + 1; k < size; k++) p.p_vx_image[k-1] = p_vx_image[k];
		p_vx_image = p.p_vx_image;
		size--;
		return true;
	}

	bool insert_image(vx_image img, int i)
	{
		pointeur_dimage p;
		p.p_vx_image = new vx_image[size + 1];
		for (int k = 0; k < i; k++) p.p_vx_image[k] = p_vx_image[k];
		p.p_vx_image[i] = img;
		for (int k = i + 1; k < size+1; k++) p.p_vx_image[k] = p_vx_image[k-1];
		p_vx_image = p.p_vx_image;
		size++;
	}

	int getsize()
	{
		return size;
	}
	
};