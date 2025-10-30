#include "commandpalette.h"
#include <QKeyEvent>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <algorithm>

CommandPalette::CommandPalette(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Command Palette"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);

    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Search edit
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText(tr("Type a command..."));
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 12px;"
        "    font-size: 14px;"
        "    border: none;"
        "    border-bottom: 1px solid #E0E0E0;"
        "    background: white;"
        "}"
    );
    layout->addWidget(searchEdit);

    // Command list
    commandList = new QListWidget(this);
    commandList->setStyleSheet(
        "QListWidget {"
        "    border: none;"
        "    background: white;"
        "    font-size: 13px;"
        "}"
        "QListWidget::item {"
        "    padding: 8px 12px;"
        "    border-bottom: 1px solid #F5F5F5;"
        "}"
        "QListWidget::item:selected {"
        "    background: #E3F2FD;"
        "    color: #1976D2;"
        "}"
        "QListWidget::item:hover {"
        "    background: #F5F5F5;"
        "}"
    );
    layout->addWidget(commandList);

    // Connect signals
    connect(searchEdit, &QLineEdit::textChanged, this, &CommandPalette::filterCommands);
    connect(commandList, &QListWidget::itemActivated, this, &CommandPalette::executeCommand);
    connect(commandList, &QListWidget::itemClicked, this, &CommandPalette::executeCommand);

    // Set dialog properties
    setMinimumWidth(600);
    setMaximumWidth(800);
    setMinimumHeight(400);
    setMaximumHeight(600);

    // Apply styling
    setStyleSheet(
        "QDialog {"
        "    background: white;"
        "    border: 1px solid #CCCCCC;"
        "    border-radius: 8px;"
        "}"
    );
}

void CommandPalette::setActions(const QList<QAction*> &actions)
{
    allActions = actions;
    populateCommands();
}

void CommandPalette::populateCommands()
{
    commandItems.clear();

    for (QAction *action : allActions) {
        if (!action->isSeparator() && action->isVisible() && !action->text().isEmpty()) {
            CommandItem item;
            item.action = action;

            // Remove & from text (used for keyboard mnemonics)
            QString text = action->text().remove('&');
            QString shortcut = action->shortcut().toString(QKeySequence::NativeText);

            // Create display text with shortcut
            if (!shortcut.isEmpty()) {
                item.displayText = QString("%1    [%2]").arg(text).arg(shortcut);
            } else {
                item.displayText = text;
            }

            // Search text (lowercase for case-insensitive search)
            item.searchText = text.toLower();

            commandItems.append(item);
        }
    }
}

void CommandPalette::show()
{
    // Repopulate commands in case they changed
    populateCommands();

    // Clear search
    searchEdit->clear();

    // Show all commands initially
    filterCommands("");

    // Select first item
    if (commandList->count() > 0) {
        commandList->setCurrentRow(0);
    }

    // Focus search edit
    searchEdit->setFocus();

    // Center on parent
    if (parentWidget()) {
        QRect parentGeometry = parentWidget()->geometry();
        int x = parentGeometry.x() + (parentGeometry.width() - width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - height()) / 2;
        move(x, y);
    }

    QDialog::show();
}

void CommandPalette::filterCommands(const QString &text)
{
    commandList->clear();

    QString pattern = text.toLower();

    if (pattern.isEmpty()) {
        // Show all commands
        for (const CommandItem &item : commandItems) {
            commandList->addItem(item.displayText);
        }
    } else {
        // Fuzzy match and score
        struct ScoredItem {
            CommandItem item;
            int score;
        };
        QList<ScoredItem> scoredItems;

        for (const CommandItem &item : commandItems) {
            if (fuzzyMatch(pattern, item.searchText)) {
                int score = fuzzyScore(pattern, item.searchText);
                scoredItems.append({item, score});
            }
        }

        // Sort by score (higher is better)
        std::sort(scoredItems.begin(), scoredItems.end(),
                  [](const ScoredItem &a, const ScoredItem &b) {
                      return a.score > b.score;
                  });

        // Add to list
        for (const ScoredItem &scored : scoredItems) {
            commandList->addItem(scored.item.displayText);
        }
    }

    // Select first item
    if (commandList->count() > 0) {
        commandList->setCurrentRow(0);
    }
}

void CommandPalette::executeCommand(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    // Find the action corresponding to this item
    QString displayText = item->text();
    for (const CommandItem &cmdItem : commandItems) {
        if (cmdItem.displayText == displayText) {
            // Trigger the action
            if (cmdItem.action && cmdItem.action->isEnabled()) {
                cmdItem.action->trigger();
            }
            break;
        }
    }

    // Close the palette
    accept();
}

void CommandPalette::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
    } else if (event->key() == Qt::Key_Down) {
        int currentRow = commandList->currentRow();
        if (currentRow < commandList->count() - 1) {
            commandList->setCurrentRow(currentRow + 1);
        }
    } else if (event->key() == Qt::Key_Up) {
        int currentRow = commandList->currentRow();
        if (currentRow > 0) {
            commandList->setCurrentRow(currentRow - 1);
        }
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QListWidgetItem *item = commandList->currentItem();
        if (item) {
            executeCommand(item);
        }
    } else {
        QDialog::keyPressEvent(event);
    }
}

bool CommandPalette::fuzzyMatch(const QString &pattern, const QString &text)
{
    if (pattern.isEmpty()) {
        return true;
    }

    int patternIdx = 0;
    int textIdx = 0;

    while (patternIdx < pattern.length() && textIdx < text.length()) {
        if (pattern[patternIdx] == text[textIdx]) {
            patternIdx++;
        }
        textIdx++;
    }

    return patternIdx == pattern.length();
}

int CommandPalette::fuzzyScore(const QString &pattern, const QString &text)
{
    if (pattern.isEmpty()) {
        return 0;
    }

    int score = 0;
    int patternIdx = 0;
    int textIdx = 0;
    int consecutiveMatches = 0;

    while (patternIdx < pattern.length() && textIdx < text.length()) {
        if (pattern[patternIdx] == text[textIdx]) {
            // Bonus for consecutive matches
            consecutiveMatches++;
            score += 10 + consecutiveMatches * 5;

            // Bonus for matching at word boundaries
            if (textIdx == 0 || text[textIdx - 1] == ' ') {
                score += 20;
            }

            patternIdx++;
        } else {
            consecutiveMatches = 0;
        }
        textIdx++;
    }

    // Penalty for longer text (prefer shorter matches)
    score -= text.length();

    return score;
}
