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
function get_status_class(string $status) : string {
    $label = '';
    switch ($status) {
        case 'OK':
            $label = 'success';
            break;
        case 'ACCESS_DENIED':
        case 'NOT_FOUND':
            $label = 'danger';
            break;
        case 'REQUESTING_CONFIGURATION':
            $label = 'info';
            break;
        default:
            $label = 'default';
            break;
    }

    return $label;
}
?>
