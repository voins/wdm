#ifndef _WDMPREFS_H
#define _WDMPREFS_H

typedef struct _Panel
{
	char *description;
	void (*destroy)(struct _Panel *);
	void (*show)(struct _Panel *);
	void (*hide)(struct _Panel *);
	void (*save)(struct _Panel *);
	void (*undo)(struct _Panel *);
	void *data;
} Panel;

void AddSectionButton(Panel *panel, const char *iconfile);

void InitTestPanel(WMWidget *win);
void InitTestPanel2(WMWidget *win);

#endif /* _WDMPREFS_H */

