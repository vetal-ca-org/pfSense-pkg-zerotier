<?php
/*
*
*/
require_once("util.inc");
require_once("globals.inc");
require_once("config.inc");
require_once("interfaces.inc");
require_once("services.inc");
require_once("service-utils.inc");

function zerotier_sync() {
	global $config;
	
	// Get the action from the request
	$action = $_REQUEST['act'] ?? '';
	
	switch ($action) {
		case 'save':
			// Save current ZeroTier state to pfSense config.xml
			zerotier_serialize_state();
			break;
			
		case 'restore':
			// Restore ZeroTier state from pfSense config.xml
			zerotier_deserialize_state();
			break;
			
		case 'clear':
			// Clear ZeroTier state from pfSense config.xml
			zerotier_clear_state();
			break;
			
		default:
			// Default sync behavior - start/stop service based on configuration
			$zerotier_config = config_get_path('installedpackages/zerotier/config/0');

			if(!($zerotier_config['enable'])) {
				if(is_process_running("zerotier")) {
					stop_service("zerotier");
				}
				return;
			}

			if(is_process_running("zerotier")) {
				stop_service("zerotier");
			}

			start_service("zerotier");
			break;
	}
}

function zerotier_install() {

	config_get_path('installedpackages/zerotier/config', array());

	$_rcfile['file'] = 'zerotier.sh';
	$_rcfile['start'] = '/usr/local/sbin/zerotier-one -d';
	$_rcfile['stop'] = '/usr/bin/killall -q zerotier-one';

	write_rcfile($_rcfile);

	$file_contents = file_get_contents("/etc/inc/util.inc");
	$file_contents = preg_replace('/(\^enc\|)/', "^zt|$1", $file_contents);
	file_put_contents("/etc/inc/util.inc", $file_contents);

	// Create Interface Group
	$found = false;

	// Check if the interface group already exists
	foreach (config_get_path('ifgroups/ifgroupentry', []) as $ifg) {
		if ($ifg['ifname'] == "pkg_zerotier") {
			$found = true;
		}
	}

	// Add the interface group if it doesn't exist
	if (!$found) {
		$ifgroupentry = array();
		$ifgroupentry['members'] = '';
		$ifgroupentry['descr'] = 'ZeroTier VPN interface group';
		$ifgroupentry['ifname'] = 'pkg_zerotier';
		// Use array_push to add the new entry to the existing array
		$ifgroups = config_get_path('ifgroups/ifgroupentry', []);
		array_push($ifgroups, $ifgroupentry);
		config_set_path('ifgroups/ifgroupentry', $ifgroups);
	}
	
	// Restore ZeroTier state from XML configuration
	zerotier_deserialize_state();
	
    /*
     * XXX: Do not remove this, otherwise the interface group may not be added properly.
     */
    // unlink_if_exists("{$g['tmp_path']}/config.cache");

	write_config("[zerotier] Package installed.");
}

function zerotier_deinstall() {

	if(is_process_running("zerotier")) {
		stop_service("zerotier");
	}

	// Serialize ZeroTier state to XML configuration before uninstalling
	zerotier_serialize_state();

	/* Remove Interface Group */

	$myid = -1;
	foreach (config_get_path('ifgroups/ifgroupentry', []) as $idx => $ifgroupentry) {
		if ($ifgroupentry['ifname'] == 'pkg_zerotier') {
			$myid = $idx;
			break;
		}
	}

	if (is_numericint($myid) && config_get_path("ifgroups/ifgroupentry/{$myid}")) {
		$members = array_filter(explode(" ", config_get_path("ifgroups/ifgroupentry/{$myid}/members")));
		foreach ($members as $ifs) {
			$realif = get_real_interface($ifs);
			if ($realif) {
				mwexec("/sbin/ifconfig {$realif} -group " . escapeshellarg(config_get_path("ifgroups/ifgroupentry/{$myid}/ifname")));
			}
		}
		config_del_path("ifgroups/ifgroupentry/{$myid}");
	}

	config_del_path('installedpackages/zerotier/config/0');
	write_config("[zerotier] Package uninstalled.");

	$file_contents = file_get_contents("/etc/rc.conf.local");
	$file_contents = str_replace("zerotier_enable=\"YES\"\n","",$file_contents);
	file_put_contents("/etc/rc.conf.local",$file_contents);

	$file_contents = file_get_contents("/etc/inc/util.inc");
	$file_contents = preg_replace('/\^zt\|(\^enc\|)/', "$1", $file_contents);
	file_put_contents("/etc/inc/util.inc", $file_contents);
	unlink_if_exists('/usr/local/etc/rc.d/zerotier.sh');

}

function zerotier_kill() {
	if (is_service_running("zerotier")) {
		stop_service("zerotier");
	}
}

function zerotier_start() {
	$zt = config_get_path('installedpackages/zerotier/config/0');
	if ($zt['enable'] == 'yes') {
		if (!is_service_running("zerotier")) {
			start_service("zerotier");
		}
	}
}

function zerotier_status() {
	$output = [];
	if (is_service_running("zerotier"))
	{
		exec("/usr/local/bin/zerotier-cli -j /status", $json);
		$output = json_decode(implode('', $json));
	}
	return $output;
}


# Used by Status Page
function zerotier_listnetworks() {
	$networks = [];
	if (is_service_running("zerotier"))
	{
		exec("/usr/local/bin/zerotier-cli -j listnetworks", $json);
		$networks = json_decode(implode('', $json));
	}
	return $networks;
}

function zerotier_listpeers() {
	$peers = [];
	if (is_service_running("zerotier")) {
		exec("/usr/local/bin/zerotier-cli -j /peer", $json);
		$peers = json_decode(implode('', $json));
	}
	return $peers;
}

function zerotier_join_network($network) {
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');
	$output = [];
	if (is_service_running("zerotier")) {
		exec("/usr/local/bin/curl -X POST --header \"X-ZT1-Auth: ${auth_secret}\" http://localhost:9993/network/${network}", $json);
		$output = json_decode(implode('', $json));
	}
	return $output;
}

function zerotier_leave_network($network) {
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');
	$output = [];
	if (is_service_running("zerotier")) {
		exec("/usr/local/bin/curl -X DELETE --header \"X-ZT1-Auth: ${auth_secret}\" http://localhost:9993/network/${network}", $json);
		$output = json_decode(implode('', $json));
	}
	return $output;
}

function zerotier_controller_listnetworks() {
	$networks = [];
	$network_list = [];

	if (is_service_running("zerotier")) {
		exec("/usr/local/bin/zerotier-cli -j /controller/network", $json);
		$network_list = json_decode(implode('', $json));
	}

	foreach($network_list as $network) {
		exec("/usr/local/bin/zerotier-cli -j /controller/network/${network}", $json2);
		$networks[] = json_decode(implode('', $json2));
		unset($json2);
	}

	return $networks;
}

function zerotier_controller_network($network) {
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');

	$output = [];
	// if (is_service_running("zerotier")) {
	// 	exec("/usr/local/bin/curl --header \"X-ZT1-Auth: ${auth_secret}\" http://localhost:9993/controller/network/${network}", $json);
	// 	$output = json_decode(implode('', $json));
	// }

	if (is_service_running("zerotier")) {
		$curl = curl_init("http://localhost:9993/controller/network/".$network);
		curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($curl, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($curl, CURLOPT_HTTPHEADER, array(
			'X-ZT1-Auth: ' .  $auth_secret
			)
		);

		$curl_out = curl_exec($curl);
		curl_close($curl);
		$output = json_decode($curl_out);
	}
	return $output;
}

function zerotier_controller_createnetwork($network, $networkID) {
	global $id;
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');
	$output = [];
	if (is_service_running("zerotier")) {
		exec("/usr/local/bin/zerotier-cli -j /status", $json);
		$status = json_decode(implode('', $json));
		$controller_id = $status->address;
		$network_id = $controller_id.$networkID;
		$post_data = json_encode($network);
		$curl = curl_init("http://localhost:9993/controller/network/".$network_id);
		curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_setopt($curl, CURLOPT_POSTFIELDS, $post_data);
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($curl, CURLOPT_HTTPHEADER, array(
			'Content-Type: application/json',
			'Content-Length: ' . strlen($post_data),
			'X-ZT1-Auth: ' .  $auth_secret
			)
		);

		$curl_out = curl_exec($curl);
		curl_close($curl);
		$output = json_decode($curl_out);
	}
	return $output;
}
function zerotier_controller_deletenetwork($id) {
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');
	$output = [];
	if (is_service_running("zerotier")) {
		exec("/usr/local/bin/curl -X DELETE --header \"X-ZT1-Auth: ${auth_secret}\" http://localhost:9993/controller/network/${id}", $json);
		$output = json_decode(implode('', $json));
	}
	return $output;
}

function zerotier_controller_network_members($network) {
	global $id;
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');
	$output = [];
	$members = [];

	if (is_service_running("zerotier")) {

		$curl = curl_init("http://localhost:9993/controller/network/".$network."/member");
		curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($curl, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($curl, CURLOPT_HTTPHEADER, array(
			'X-ZT1-Auth: ' .  $auth_secret
			)
		);

		$curl_out = curl_exec($curl);
		curl_close($curl);
		$members = json_decode($curl_out);


		foreach($members as $key => $value) {
			unset($curl);
			unset($response);
			$curl = curl_init("http://localhost:9993/controller/network/".$network."/member/".$key);
			curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
			curl_setopt($curl, CURLOPT_FOLLOWLOCATION, true);
			curl_setopt($curl, CURLOPT_HTTPHEADER, array(
				'X-ZT1-Auth: ' .  $auth_secret
				)
			);

			$response = curl_exec($curl);
			curl_close($curl);
			$output[] = json_decode($response);
		}
	}
	return $output;
}
function zerotier_controller_member_toggle($network, $member, $key) {
	$auth_secret = file_get_contents('/var/db/zerotier-one/authtoken.secret');
	$output = [];


	if (is_service_running("zerotier")) {
		$curl = curl_init("http://localhost:9993/controller/network/".$network."/member/".$member);
		curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($curl, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($curl, CURLOPT_HTTPHEADER, array(
			'X-ZT1-Auth: ' .  $auth_secret
			)
		);

		$response = curl_exec($curl);
		curl_close($curl);
		$current = json_decode($response, true);
		$new = !($current[$key]);

		$post_data = json_encode([$key => $new]);
		$curl = curl_init("http://localhost:9993/controller/network/".$network."/member/".$member);
		curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_setopt($curl, CURLOPT_POSTFIELDS, $post_data);
			curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
			curl_setopt($curl, CURLOPT_HTTPHEADER, array(
				'Content-Type: application/json',
				'Content-Length: ' . strlen($post_data),
				'X-ZT1-Auth: ' .  $auth_secret
				)
			);

		$response = curl_exec($curl);
		curl_close($curl);
		$output[] = json_decode($response);
	}
	return $output;
}
// Interface Functions

function zt_get_pfsense_interface_info($portDeviceName): pfInterface {
	$ifList = get_configured_interface_list_by_realif(true);
	$ifDesc = get_configured_interface_with_descr(true);

	if($ifList[$portDeviceName]) {
		return new pfInterface($ifList[$portDeviceName], $ifDesc[$ifList[$portDeviceName]]); 
	}
	else {
		return new pfInterface();
	}
}

class pfInterface {
	public $interface;
	public $description;

	public function __construct(string $interface = "", string $description = "")
	{
		$this->interface = $interface;
		$this->description = $description;
	}
}

// Helper Functions

/**
 * get_status_class
 *
 * @param  string $status
 * @return string
 */
function get_status_class($status) {
	switch($status) {
		case "OK":
			return "success";
		case "ACCESS_DENIED":
			return "danger";
		case "NOT_FOUND":
			return "warning";
		case "PORT_ERROR":
			return "danger";
		default:
			return "info";
	}
}

/**
 * Serialize ZeroTier state to pfSense config.xml
 * This function saves the current ZeroTier state to pfSense's main configuration
 */
function zerotier_serialize_state() {
	global $config;
	
	// Create the state array for config.xml
	$zerotier_state = array();
	
	// Save basic configuration (already in config.xml)
	if (isset($config['installedpackages']['zerotier']['config'][0])) {
		$zerotier_state['config'] = $config['installedpackages']['zerotier']['config'][0];
	}
	
	// Save ZeroTier identity and network data as base64 encoded strings
	if (is_dir('/var/db/zerotier-one')) {
		// Save identity files
		if (file_exists('/var/db/zerotier-one/identity.public')) {
			$zerotier_state['identity']['public'] = base64_encode(file_get_contents('/var/db/zerotier-one/identity.public'));
		}
		if (file_exists('/var/db/zerotier-one/identity.secret')) {
			$zerotier_state['identity']['secret'] = base64_encode(file_get_contents('/var/db/zerotier-one/identity.secret'));
		}
		if (file_exists('/var/db/zerotier-one/authtoken.secret')) {
			$zerotier_state['identity']['authtoken'] = base64_encode(file_get_contents('/var/db/zerotier-one/authtoken.secret'));
		}
		
		// Save local configuration
		if (file_exists('/var/db/zerotier-one/local.conf')) {
			$zerotier_state['local_conf'] = base64_encode(file_get_contents('/var/db/zerotier-one/local.conf'));
		}
		
		// Save network configurations
		$networks_dir = '/var/db/zerotier-one/networks.d';
		if (is_dir($networks_dir)) {
			$zerotier_state['networks'] = array();
			$network_files = glob($networks_dir . '/*.conf');
			foreach ($network_files as $network_file) {
				$network_id = basename($network_file, '.conf');
				$zerotier_state['networks'][$network_id] = base64_encode(file_get_contents($network_file));
			}
		}
		
		// Save peer information
		$peers_dir = '/var/db/zerotier-one/peers.d';
		if (is_dir($peers_dir)) {
			$zerotier_state['peers'] = array();
			$peer_files = glob($peers_dir . '/*.conf');
			foreach ($peer_files as $peer_file) {
				$peer_id = basename($peer_file, '.conf');
				$zerotier_state['peers'][$peer_id] = base64_encode(file_get_contents($peer_file));
			}
		}
		
		// Save controller data if exists
		$controller_dir = '/var/db/zerotier-one/controller.d';
		if (is_dir($controller_dir)) {
			$zerotier_state['controller'] = array();
			$controller_files = glob($controller_dir . '/*.conf');
			foreach ($controller_files as $controller_file) {
				$controller_id = basename($controller_file, '.conf');
				$zerotier_state['controller'][$controller_id] = base64_encode(file_get_contents($controller_file));
			}
		}
	}
	
	// Save the state to pfSense config.xml
	$config['installedpackages']['zerotier']['state'] = $zerotier_state;
	write_config("[zerotier] State serialized to config.xml.");
	
	return true;
}

/**
 * Deserialize ZeroTier state from pfSense config.xml
 * This function restores the ZeroTier state from pfSense's main configuration
 */
function zerotier_deserialize_state() {
	global $config;
	
	if (!isset($config['installedpackages']['zerotier']['state'])) {
		return false;
	}
	
	$zerotier_state = $config['installedpackages']['zerotier']['state'];
	
	// Ensure the ZeroTier directory exists
	if (!is_dir('/var/db/zerotier-one')) {
		mkdir('/var/db/zerotier-one', 0700, true);
	}
	
	// Restore identity files
	if (isset($zerotier_state['identity'])) {
		if (isset($zerotier_state['identity']['public'])) {
			file_put_contents('/var/db/zerotier-one/identity.public', base64_decode($zerotier_state['identity']['public']));
			chmod('/var/db/zerotier-one/identity.public', 0600);
		}
		if (isset($zerotier_state['identity']['secret'])) {
			file_put_contents('/var/db/zerotier-one/identity.secret', base64_decode($zerotier_state['identity']['secret']));
			chmod('/var/db/zerotier-one/identity.secret', 0600);
		}
		if (isset($zerotier_state['identity']['authtoken'])) {
			file_put_contents('/var/db/zerotier-one/authtoken.secret', base64_decode($zerotier_state['identity']['authtoken']));
			chmod('/var/db/zerotier-one/authtoken.secret', 0600);
		}
	}
	
	// Restore local configuration
	if (isset($zerotier_state['local_conf'])) {
		file_put_contents('/var/db/zerotier-one/local.conf', base64_decode($zerotier_state['local_conf']));
		chmod('/var/db/zerotier-one/local.conf', 0644);
	}
	
	// Restore network configurations
	if (isset($zerotier_state['networks']) && is_array($zerotier_state['networks'])) {
		$networks_dir = '/var/db/zerotier-one/networks.d';
		if (!is_dir($networks_dir)) {
			mkdir($networks_dir, 0700, true);
		}
		foreach ($zerotier_state['networks'] as $network_id => $network_data) {
			file_put_contents($networks_dir . '/' . $network_id . '.conf', base64_decode($network_data));
			chmod($networks_dir . '/' . $network_id . '.conf', 0600);
		}
	}
	
	// Restore peer information
	if (isset($zerotier_state['peers']) && is_array($zerotier_state['peers'])) {
		$peers_dir = '/var/db/zerotier-one/peers.d';
		if (!is_dir($peers_dir)) {
			mkdir($peers_dir, 0700, true);
		}
		foreach ($zerotier_state['peers'] as $peer_id => $peer_data) {
			file_put_contents($peers_dir . '/' . $peer_id . '.conf', base64_decode($peer_data));
			chmod($peers_dir . '/' . $peer_id . '.conf', 0600);
		}
	}
	
	// Restore controller data
	if (isset($zerotier_state['controller']) && is_array($zerotier_state['controller'])) {
		$controller_dir = '/var/db/zerotier-one/controller.d';
		if (!is_dir($controller_dir)) {
			mkdir($controller_dir, 0700, true);
		}
		foreach ($zerotier_state['controller'] as $controller_id => $controller_data) {
			file_put_contents($controller_dir . '/' . $controller_id . '.conf', base64_decode($controller_data));
			chmod($controller_dir . '/' . $controller_id . '.conf', 0600);
		}
	}
	
	// Set proper ownership
	chown('/var/db/zerotier-one', 'root');
	chgrp('/var/db/zerotier-one', 'wheel');
	
	return true;
}

/**
 * Clear ZeroTier state from pfSense config.xml
 * This function removes the serialized state from pfSense's main configuration
 */
function zerotier_clear_state() {
	global $config;
	
	if (isset($config['installedpackages']['zerotier']['state'])) {
		unset($config['installedpackages']['zerotier']['state']);
		write_config("[zerotier] State cleared from config.xml.");
	}
	
	return true;
}

/**
 * Save ZeroTier state to config.xml
 * This is the main function called by pfSense's save mechanism
 */
function zerotier_save_state() {
	return zerotier_serialize_state();
}
?>
