/*
 * Gesture threshold, device parameters
 */

#include <stdio.h>

/* device parameters */
static const float m_device_xres = 1024.0;	/* panel x resolution (pixel) */
static const float m_device_yres = 600.0;	/* panel y resolution (pixel) */
static const float m_phys_xsize  = 222.72;	/* panel size width (mm) */
static const float m_phys_ysize  = 125.25;	/* panel size height (mm) */
static const float m_mapped_xres = 2048.0;	/* mapping x resolution (pixel) */
static const float m_mapped_yres = 2048.0;	/* mapping y resolution (pixel) */

float m_scale_x = 1.0;				/* mapping x pixel scale */
float m_scale_y = 1.0;				/* mapping y pixel scale */
float m_scale_ppm_x = 1.0;			/* pixel per mm */
float m_scale_ppm_y = 1.0;			/* pixel per mm */

/* flick threshold */
float flick_dist_min_threshold = 10.0;     /* min distance */
float flick_dist_max_threshold = 200.0;    /* max distance */
float flick_velo_min_threshold = 150.0;      /* ave. velocity min (mm/s)*/
float flick_time_min_threshold = 50.0;     /* min time */
float flick_time_max_threshold = 300.0;    /* max time */

/* pinching threshold */
float pinch_dist_min_threshold = 4.0;      /* min distance */

void gesture_init()
{
	m_scale_x = m_mapped_xres / m_device_xres;
	m_scale_y = m_mapped_yres / m_device_yres;
	m_scale_ppm_x = m_device_xres / m_phys_xsize;
	m_scale_ppm_y = m_device_yres / m_phys_ysize;
	flick_velo_min_threshold /= 1000;	/* mm/ms */

	fprintf(stdout, "%s() - device  resolution %.1f x %.1f\n",
			__func__, m_device_xres, m_device_yres);
	fprintf(stdout, "%s() - mapping resolution %.1f x %.1f\n",
			__func__, m_mapped_xres, m_mapped_yres);
	fprintf(stdout, "%s() - mapping scale x:%.2f, y:%.2f\n",
			__func__, m_scale_x, m_scale_y);
	fprintf(stdout, "%s() - x:%.2f pixel/mm, y:%.2f pixel/mm\n",
			__func__, m_scale_ppm_x, m_scale_ppm_y);
}
/* EOF */
