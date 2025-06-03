#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>

static int __init hmbird_patch_init(void)
{
    struct device_node *ver_np;
    const char *type;
    int ret;

    ver_np = of_find_node_by_path("/soc/oplus,hmbird/version_type");
    if (!ver_np) {
        pr_info("hmbird_patch: version_type node not found, skipping patch.\n");
        return 0;
    }

    ret = of_property_read_string(ver_np, "type", &type);
    if (ret) {
        pr_info("hmbird_patch: type property not found, skipping patch.\n");
        of_node_put(ver_np);
        return 0;
    }

    if (strcmp(type, "HMBIRD_OGKI")) {
        pr_info("hmbird_patch: type is not HMBIRD_OGKI, skipping patch. Current type: %s\n", type);
        of_node_put(ver_np);
        return 0;
    }

    struct property *prop = of_find_property(ver_np, "type", NULL);
    if (prop) {
        char *new_value = kmalloc(strlen("HMBIRD_GKI") + 1, GFP_KERNEL);
        if (!new_value) {
            pr_err("hmbird_patch: kmalloc for new_value failed\n");
            of_node_put(ver_np);
            return -ENOMEM;
        }
        strcpy(new_value, "HMBIRD_GKI");

        struct property *new_prop_st = kmalloc(sizeof(*new_prop_st), GFP_KERNEL);
        if (!new_prop_st) {
            pr_err("hmbird_patch: kmalloc for new_prop_st failed\n");
            kfree(new_value);
            of_node_put(ver_np);
            return -ENOMEM;
        }
        memcpy(new_prop_st, prop, sizeof(*new_prop_st)); 
        new_prop_st->value = new_value;
        new_prop_st->length = strlen("HMBIRD_GKI") + 1;

        if (of_remove_property(ver_np, prop) != 0) {
           pr_err("hmbird_patch: of_remove_property failed\n");
           kfree(new_value); 
           kfree(new_prop_st); 
           of_node_put(ver_np); 
           return -EINVAL; 
        }
        
        if (of_add_property(ver_np, new_prop_st) !=0) {
           pr_err("hmbird_patch: of_add_property failed\n");
           kfree(new_value); 
           kfree(new_prop_st); 
           of_node_put(ver_np);
           return -EINVAL; 
        }
        pr_info("hmbird_patch: success from HMBIRD_OGKI to HMBIRD_GKI\n");
   } else {
        pr_info("hmbird_patch: 'type' property not found (struct property), skipping.\n");
   }
   of_node_put(ver_np); 
   return 0;
}
early_initcall(hmbird_patch_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("reigadegr"); 
MODULE_DESCRIPTION("Forcefully convert HMBIRD_OGKI to HMBIRD_GKI.");
