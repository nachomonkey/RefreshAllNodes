# Refresh All Nodes

*Unreal Engine plugin that refreshes all blueprint nodes in every blueprint* 

## Installation

1. Either clone the repository or extract the ZIP file into your project's Plugins directory.

2. Restart the editor.

## Usage

This plugin adds the **Refresh All Blueprint Nodes** button to the Blueprints toolbar menu as shown below:

![The button is shown in the Blueprints toolbar menu](docs/MenuButton.png)

Clicking the button will refresh all nodes in all of your blueprints. It performs the same action as manually using "Refresh All nodes" on each blueprint.

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

#### Unreal Engine Versions

* 4.25
* 4.26.1

# License

This software is under the MIT License. See the [LICENSE](https://github.com/nachomonkey/RefreshAllNodes/blob/master/LICENSE) file for the full license.