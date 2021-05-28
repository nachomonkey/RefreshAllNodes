# Refresh All Nodes

*Unreal Engine plugin that refreshes all blueprint nodes in every blueprint* 

## Installation

### Pre-compiled (recommended)

*Pre-compiled released of this plugin are supplied for Windows 64-bit.*

Download a release that matches your version of Unreal Engine [from here](https://github.com/nachomonkey/RefreshAllNodes/releases), and extract it into your project's Plugins directory.

Relaunch the editor.

### Compiling from source

Clone or [download](https://github.com/nachomonkey/RefreshAllNodes/releases/tag/v1.1) the repository into your project's Plugins directory, and relaunch the editor.

A message should appear asking to compile the plugin.

## Usage

This plugin adds the **Refresh All Blueprint Nodes** button to the Blueprints toolbar menu as shown below:

![The button is shown in the Blueprints toolbar menu](docs/NewMenuButton.png)

Clicking the button will refresh all nodes in all of your blueprints. It performs the same action as manually using "Refresh All nodes" on each blueprint.

Important Notice: Do not use this plugin to fix problems caused by the Hot Reload, as it can cause node breakages and modify variable types. Instead of using this plugin, try restarting the editor.


### Configuration

The plugin can be configured under **Project Settings ->  Plugins -> Refresh All Nodes**

![Configuration](docs/Configuration.png)

* Refresh Level Blueprints: If checked, the plugin will search for level blueprints. Don't use if you don't want your all levels being opened and saved.
* Refresh Game Blueprints: If checked, the plugin will refresh blueprints found in the project's Content folder
* Refresh Engine Blueprints: If checked, the plugin will refresh blueprints found in the engine's Content folder
* Additional Blueprint Paths: Array of additional paths to search in. Most useful for plugins. Add the name of the plugin to refresh its blueprints.
* Exclude Blueprint Paths: Array of paths to not refresh blueprints in. Useful for excluding blueprints that are expensive to load.

## Compatibility

The plugin has been tested on Linux and Windows.

It has been tested with the following versions:

* 4.25.*x*
* 4.26.*x*
* 5.0.*x*

## Limitations

This plugin's ability to refresh nodes is limited to Unreal Engine's built-in "Refresh All Nodes" function. The purpose of this plugin is only to provide an easy way to perform such action on all Blueprint assets. This plugin is not responsible for any damage to Blueprints or any data loss.

# License

This software is under the MIT License. See the [LICENSE](https://github.com/nachomonkey/RefreshAllNodes/blob/master/LICENSE) file for the full license.
