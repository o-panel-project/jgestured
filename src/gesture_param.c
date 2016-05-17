/*
 * Gesture threshold, device parameters
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

/* device parameters */
static float m_screen_xres = 1024.0;	/* panel x resolution (pixel) */
static float m_screen_yres = 600.0;		/* panel y resolution (pixel) */
static float m_device_xres = 1024.0;	/* panel x resolution (pixel) */
static float m_device_yres = 600.0;		/* panel y resolution (pixel) */
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

void get_scr_resolution()
{
	struct fb_var_screeninfo vinfo;
	int fd = open("/dev/fb", O_RDWR);
	if (fd < 0)
		return;
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		m_screen_xres = vinfo.xres;
		m_screen_yres = vinfo.yres;
	}
	close(fd);
	return;
}

#define GET_RESOLUTION_FROM_SYSFS
#undef  GET_RESOLUTION_FROM_IOCTL
enum {
	MMS_IOCTL_FW_SIZE = 0xA1,
	MMS_IOCTL_FW_DATA,
	MMS_IOCTL_FW_UPDATE,
	MMS_IOCTL_FW_UPDATE_FORCE,
	MMS_IOCTL_GET_VERSION,
#if defined(GET_RESOLUTION_FROM_IOCTL)
	MMS_IOCTL_GET_RESOLUTION_X,
	MMS_IOCTL_GET_RESOLUTION_Y,
#endif
};
#if defined(GET_RESOLUTION_FROM_IOCTL)
#define	IOCTL_GET_RESOLUTION_X	_IOR('W', MMS_IOCTL_GET_RESOLUTION_X, unsigned long*)
#define	IOCTL_GET_RESOLUTION_Y	_IOR('W', MMS_IOCTL_GET_RESOLUTION_Y, unsigned long*)
#endif

#if defined(GET_RESOLUTION_FROM_SYSFS)
#define SYSFS_MMS_TS "/sys/devices/platform/omap/omap_i2c.3/i2c-3/3-0048/"
#define INFO_RESOLUTION_NAME	"mms_ts_resolution"
#endif

void get_tp_resolution()
{
#if defined(GET_RESOLUTION_FROM_SYSFS)
	FILE *fp;
	int x, y;
	if ((fp = fopen(SYSFS_MMS_TS INFO_RESOLUTION_NAME, "r")) == NULL) {
		perror("fopen");
		return;
	}
	if (fscanf(fp, "%dx%d\n", &x, &y) == EOF) {
		perror("fscanf");
		return;
	}
	fclose(fp);
	m_device_xres = x;
	m_device_yres = y;
#elif defined(GET_RESOLUTION_FROM_IOCTL)
	int x, y;
	int fd = open("/dev/mms_ts", O_RDWR);
	if (fd < 0)
		return;
	if (ioctl(fd, IOCTL_GET_RESOLUTION_X, (unsigned long*)&x) == 0) {
		if (ioctl(fd, IOCTL_GET_RESOLUTION_Y, (unsigned long*)&y) == 0) {
			m_device_xres = x;
			m_device_yres = y;
		}
	}
	close(fd);
#endif
	return;
}

void gesture_init()
{
	get_scr_resolution();
	get_tp_resolution();
	m_scale_x = m_mapped_xres / m_device_xres;
	m_scale_y = m_mapped_yres / m_device_yres;
	m_scale_ppm_x = m_device_xres / m_phys_xsize;
	m_scale_ppm_y = m_device_yres / m_phys_ysize;
	flick_velo_min_threshold /= 1000;	/* mm/ms */

	fprintf(stdout, "%s() - screen  resolution %.1f x %.1f\n",
			__func__, m_screen_xres, m_screen_yres);
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
