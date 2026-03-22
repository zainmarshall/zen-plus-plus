const vscode = require("vscode");

function activate(context) {
  const run = vscode.commands.registerCommand("zenpp.run", () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) return;

    const file = editor.document.fileName;
    if (!file.endsWith(".zpp")) {
      vscode.window.showErrorMessage("Not a .zpp file");
      return;
    }

    // Save before running
    editor.document.save().then(() => {
      let terminal = vscode.window.terminals.find((t) => t.name === "Zen++");
      if (!terminal) {
        terminal = vscode.window.createTerminal("Zen++");
      }
      terminal.show();
      terminal.sendText(`zenpp "${file}"`);
    });
  });

  context.subscriptions.push(run);
}

function deactivate() {}

module.exports = { activate, deactivate };
