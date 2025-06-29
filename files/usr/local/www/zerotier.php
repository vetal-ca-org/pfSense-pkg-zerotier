<?php
require_once("config.inc");
require_once("guiconfig.inc");
require_once("zerotier.inc");

$pgtitle = array(gettext("VPN"), gettext("Zerotier"), gettext("Configuration"));
$pglinks = array("", "pkg_edit.php?xml=zerotier.xml", "@self");
require("head.inc");

$tab_array = array();
$tab_array[] = array(gettext("Networks"), false, "zerotier_networks.php");
$tab_array[] = array(gettext("Peers"), false, "zerotier_peers.php");
$tab_array[] = array(gettext("Controller"), false, "zerotier_controller.php");
$tab_array[] = array(gettext("Configuration"), true, "zerotier.php");
add_package_tabs("Zerotier", $tab_array);
display_top_tabs($tab_array);

// Display any error or success messages
if (isset($input_errors)) {
    print_input_errors($input_errors);
}
if (isset($savemsg)) {
    print_info_box($savemsg);
}

if (!is_array($config['installedpackages']['zerotier'])) {
    $config['installedpackages']['zerotier'] = array();
}

if (!is_array($config['installedpackages']['zerotier']['config'])) {
    $config['installedpackages']['zerotier']['config'] = array();
}

if($_POST['save']) {
    if(isset($_POST['enable'])) {
        $config['installedpackages']['zerotier']['config'][0]['enable'] = 'yes';
    }
    else {
        $config['installedpackages']['zerotier']['config'][0]['enable'] = NULL;
    }

    if(isset($_POST['enableExperimental'])) {
        $config['installedpackages']['zerotier']['config'][0]['experimental'] = 'yes';
    }
    else {
        $config['installedpackages']['zerotier']['config'][0]['experimental'] = NULL;
    }
  
    // Write configuration to config.xml
    write_config(gettext("ZeroTier configuration updated."));
    
    // Trigger resync to handle service management and state serialization
    header("Location: pkg_edit.php?xml=zerotier.xml");
    exit;
}

// Handle state management actions
if(isset($_REQUEST['act'])) {
    $act = $_REQUEST['act'];
    
    switch($act) {
        case 'save_state':
            if(zerotier_save_state()) {
                $savemsg = gettext("ZeroTier state saved to config.xml successfully.");
            } else {
                $input_errors[] = gettext("Failed to save ZeroTier state to config.xml.");
            }
            break;
            
        case 'restore_state':
            if(zerotier_deserialize_state()) {
                $savemsg = gettext("ZeroTier state restored from config.xml successfully.");
            } else {
                $input_errors[] = gettext("Failed to restore ZeroTier state from config.xml.");
            }
            break;
            
        case 'clear_state':
            if(zerotier_clear_state()) {
                $savemsg = gettext("ZeroTier state cleared from config.xml successfully.");
            } else {
                $input_errors[] = gettext("Failed to clear ZeroTier state from config.xml.");
            }
            break;
    }
}

if ($config['installedpackages']['zerotier']['config'][0]['enable'] != 'yes' || !is_service_running("zerotier")) {
    print_info_box(gettext("The Zerotier service is not running."), "warning", false);
}


$enable['mode'] = $config['installedpackages']['zerotier']['config'][0]['enable'];
$enable['experimental'] = $config['installedpackages']['zerotier']['config'][0]['experimental'];

if ($config['installedpackages']['zerotier']['config'][0]['enable'] == 'yes' && is_service_running("zerotier")) {
    $status = zerotier_status();
}
?>
<div class="panel panel-default">
    <div class="panel-heading"><h2 class="panel-title">Address: <?php print($status->address); ?></h2></div>
    <div class="panel-body">
        <dl class="dl-horizontal">
        <dt><?php print(gettext("Version")); ?><dt><dd><?php print($status->version) ?></dd>
        </dl>
    </div>
</div>

<?php

$form = new Form();
$section = new Form_Section('Enable Zerotier');
$section->addInput(new Form_Checkbox(
                'enable',
                'Enable',
                'Enable zerotier client and controller.',
                $enable['mode']
            ));
$form->add($section);
$section = new Form_Section('Enable Experimental Options');
$section->addInput(new Form_Checkbox(
                'enableExperimental',
                'Enable',
                'Enable zerotier client and controller experimental fields.',
                $enable['experimental']
            ))->setHelp('This will enable all experimental field options to be displayed and proccessed.');
$form->add($section);

// Add Save State section
$section = new Form_Section('State Management');
$section->addInput(new Form_Button(
    'save_state',
    'Save State',
    'Save current ZeroTier state to configuration',
    'btn-primary'
))->setHelp('Save the current ZeroTier identity, networks, and configuration to pfSense config.xml. This state will be included in pfSense backups and restored automatically.');

$section->addInput(new Form_Button(
    'restore_state',
    'Restore State',
    'Restore ZeroTier state from configuration',
    'btn-info'
))->setHelp('Restore ZeroTier state from pfSense config.xml.');

$section->addInput(new Form_Button(
    'clear_state',
    'Clear State',
    'Clear saved ZeroTier state from configuration',
    'btn-warning'
))->setHelp('Remove saved ZeroTier state from pfSense config.xml.');

$form->add($section);

// Handle button actions
if($_POST['save_state']) {
    header("Location: pkg_edit.php?xml=zerotier.xml&act=save");
    exit;
}

if($_POST['restore_state']) {
    header("Location: pkg_edit.php?xml=zerotier.xml&act=restore");
    exit;
}

if($_POST['clear_state']) {
    header("Location: pkg_edit.php?xml=zerotier.xml&act=clear");
    exit;
}

print($form);
include("foot.inc");
?>
