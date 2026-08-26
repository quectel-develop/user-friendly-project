---
name: uniknect-c-code-generate
description: "Use when the user asks to generate UniKnect C applications, user_main integration, Quectel SDK demos, builds, or ST-Link flashing. Reads quectel and apps/example as references, reuses an existing entry file only when it is under apps but outside example/test, places every new non-entry code file in a newly created apps subdirectory, and never writes to apps/example or apps/test."
---

# UniKnect C Code Project

## Purpose

Use this skill when the user asks to create, generate, implement, or modify UniKnect C code in an existing UniKnect project under the current root.

This skill must:

- locate the existing project root under the current root
- search `quectel` for SDK interfaces related to the user requirement
- use those verified interfaces to search `apps/example` for SDK usage examples
- combine the SDK definitions and example usage when generating final code
- prefer a verified unified SDK API over an AT-command implementation of the same behavior
- create a new feature-code directory under `apps` for all newly generated non-entry code
- write and modify code only under `apps`
- reuse an existing `user/user_main.c` entry file when a valid one exists under `apps` outside `example` and `test`
- place every generated non-entry code file inside the new feature directory
- treat `apps/example` and `apps/test` as read-only protected directories
- enforce a fixed C entry function named `user_main`
- keep the existing `user_main` function signature unchanged when a valid existing entry file is reused
- add required code to `user_main.c` and remove code that is unused by the requested feature
- keep test, demo-only, and self-test logic out of `user_main.c`
- generate concise, readable code whose control flow directly matches the user requirement
- generate a `README.md` file in the new feature directory
- always show separate interactive confirmations for `build.bat config`, `build.bat all`, and `build.bat download`
- process the three command stages in `config`, `all`, then `download` order using Windows `cmd`
- automatically repair generated code after code-related compile failures, with at most 5 repair attempts
- show the separate download confirmation after every successful compile
- verify an ST-Link device before running `build.bat download`
- finish only after generation, build handling, and flash handling are complete

## When To Use

Use this skill when the user asks to create, generate, build, implement, or modify UniKnect C code, demos, modules, or application files in an existing project.

Do not use this skill for:

- text-only conceptual questions with no coding intent
- non-C language requests
- requests that only need explanation with no file changes
- requests for a new SDK or complete UniKnect repository scaffold

## Core Workflow

Follow this workflow for UniKnect C code tasks:

1. Read the user requirement and identify the requested features.
2. Locate the existing project under the current root and validate its root layout.
3. Search `quectel` for the SDK definitions related to the requested features.
4. Confirm exact headers, APIs, macros, types, signatures, and required parameters.
5. Determine whether each requested operation has a verified unified SDK API; when it does, select that API and reject duplicate AT-command logic.
6. Use the selected SDK symbols as anchors to search `apps/example` for usage examples.
7. Compare the SDK definitions with the examples and derive one correct implementation path for each requirement.
8. Locate an existing entry file under `apps`, excluding every path under `example` and `test`.
9. Create a TODO plan and a uniquely named new feature-code directory under `apps`.
10. Generate all non-entry code and `README.md` in the new directory; update only the valid existing entry file when integration is required.
11. Preserve the `user_main` entry name and signature; keep it free of test code and limit it to production integration.
12. Validate requirement coverage, API-versus-AT exclusivity, control flow, and code clarity against the SDK and examples.
13. Immediately show a button dialog asking whether to execute the config stage.
14. If config is selected, show one parameter dialog containing the two config inputs and a `Skip Parameters` option, then run the selected `build.bat config` form.
15. After config is successful or explicitly skipped, show a separate button dialog asking whether to execute `build.bat all`.
16. If build is selected, run `build.bat all`; repair generated-code failures and rebuild automatically, up to 5 code-repair attempts.
17. After `build.bat all` succeeds, show a separate button dialog asking whether to execute `build.bat download`.
18. If download is selected, detect an ST-Link device and run `build.bat download` only when the device is present.
19. Finish after all required generation, compile, and flash steps have been handled.

Do not stop after planning or research.

## Absolute Code Write Boundary

All code creation and code modification performed by this skill must stay under:

```text
<project>/apps/
```

For a generation request, only these two targets may be writable:

```text
1. <project>/apps/<new_feature_directory>/
2. <existing_entry_file_under_apps>
```

Rules:

- create every new code file inside the new feature directory
- the existing entry file is the only code file that may be modified outside the new feature directory
- the existing entry file must be under `apps` and must not be under `apps/example` or `apps/test`
- never create a new entry file inside `apps/example` or `apps/test`
- treat every path outside `<project>/apps` as read-only
- do not create or modify C source files, header files, code configuration files, linker files, build scripts, SDK files, or application code outside `apps`
- do not modify `<project>/build.bat`, `<project>/quectel`, or any other directory outside `apps`
- do not modify another existing application directory under `apps`, except for the single validated existing entry file
- continue to treat `apps/example` and `apps/test` as protected read-only directories
- use files outside the writable scope only for searching, reading, interface verification, and structural reference
- if integration would require a code change outside the writable scope, stop and report the required external change instead of applying it
- automatic compile repairs must obey the same writable scope
- files produced automatically by `build.bat config`, `build.bat all`, or `build.bat download` are command outputs and are not agent-authored code changes; do not manually edit those outputs
- before each edit, verify that the target is either inside `<project>/apps/<new_feature_directory>/` or is the single validated existing entry file
- before finishing, inspect all agent-authored changed paths and confirm that they contain only the new feature directory and, when needed, the validated existing entry file

## Existing Project Detection Rules

The project folder name is not fixed. Search under the current root for an existing project with this minimum layout:

```text
<project>/
├── build.bat
├── quectel/
└── apps/
	└── example/
```

Rules:

- do not assume the project folder name
- require `build.bat`, `quectel`, and `apps/example` directly under the project root
- if exactly one valid project is found, use it
- if multiple valid projects are found, ask the user to select one through an interactive prompt
- if no valid project is found, stop and report the missing required layout
- do not create or replace `build.bat`, `quectel`, `apps`, or `example`

## Dual-Source Code Generation Rules

Final code must be grounded in both of these local sources:

1. SDK interface code under `<project>/quectel`
2. SDK usage examples under `<project>/apps/example`

SDK lookup rules:

- derive search terms from the user's requested feature
- search `quectel` first
- identify exact headers, functions, macros, enums, structs, callback signatures, initialization order, and return-value conventions
- treat `quectel` as the source of truth for interface definitions
- do not invent an API that cannot be found there

Example lookup rules:

- search `apps/example` only after the relevant SDK symbols have been identified
- use verified SDK symbol names and headers as the example search anchors
- inspect examples that most closely match the requested feature
- use examples to determine call order, setup, cleanup, error handling, and project integration patterns
- do not copy unrelated example features into the generated application

Combination rules:

- final code must match the interface shape in `quectel`
- final code should follow validated usage patterns in `apps/example`
- for each requested operation, determine whether the SDK exposes a unified callable API that implements that operation
- when a unified SDK API exists, use only that API for the operation
- do not also send AT commands, call an AT transport, or retain an AT fallback for behavior already provided by the selected API
- do not generate two implementations of the same logic where one path calls an SDK API and another path performs the same operation through AT commands
- use an AT-command path only when no suitable unified SDK API can be verified and the required AT command and transport usage are themselves verified from local code
- if the user explicitly requests AT while a unified API exists, report the conflict and ask for confirmation before overriding API-only behavior
- if an example conflicts with the current SDK definition, prefer the current SDK definition and adapt the example pattern
- if no relevant SDK definition is found, stop and report the unresolved interface instead of guessing
- if an SDK definition exists but no matching example is found, use nearby verified SDK consumers when available and report the missing example coverage

## New Feature Directory Rules

Every generation request must create one new feature-code directory under `apps` for all newly generated files except an existing entry file.

Required shape:

```text
<project>/
└── apps/
	├── <existing_safe_application>/
	│   └── user/
	│       └── user_main.c
	└── <new_feature_directory>/
		├── README.md
		├── <feature_source>.c
		└── <feature_header>.h
```

Rules:

- derive a concise new feature-directory name from the user requirement
- create exactly one new feature directory directly under `apps`
- all newly generated code files other than an entry file must be placed in this new directory
- do not create non-entry code directly in `apps`
- do not place new non-entry code beside an existing `user_main.c` unless that location is the new feature directory
- do not write generated code into `quectel`
- do not modify `apps/example`
- do not modify `apps/test`
- never create, overwrite, rename, move, or delete files or directories under `apps/example` or `apps/test`
- do not write generated files directly into `apps`
- if the derived name already exists, choose a clear unique name or ask the user before overwriting
- every generated source or header file other than the entry file must be created inside `<project>/apps/<new_feature_directory>/`
- non-entry code must not be created in another existing application directory
- README and any required metadata for the generated feature must also be created in the new feature directory
- examples may inform file contents and structure, but their paths must never be used as output paths

Protected-directory rules:

- `apps/example` may be read and searched only as a usage-reference source
- `apps/test` must not be used as a generated-code destination
- neither protected directory may be edited for integration, compilation repair, or flashing
- before every file-creation or file-edit tool call, resolve and normalize the absolute target path
- compare protected paths case-insensitively on Windows and reject both the protected directory itself and every descendant path
- if the normalized target begins with `<project>/apps/example/` or `<project>/apps/test/`, do not call an editing tool for that target
- when a proposed output path is rejected, create or select the new feature directory and redirect newly generated code there; never redirect into another existing application
- never select `apps/example/**/user_main.c` or `apps/test/**/user_main.c` as the reusable entry file
- never copy a generated result back into the example or test path from which a reference was read
- if the requested application name resolves to `example` or `test`, choose another descriptive name
- before finishing generation, inspect the changed-file set and confirm that neither protected directory contains changes

## Existing Entry File Rules

The entry function file may reuse an existing file. Locate it under `apps` before generating other code.

Valid entry path shape:

```text
<project>/apps/<safe_existing_application>/user/user_main.c
```

Invalid entry paths:

```text
<project>/apps/example/**/user_main.c
<project>/apps/test/**/user_main.c
```

The entry function must remain named exactly:

```c
user_main
```

Rules:

- search for an existing `user_main.c` only under `apps`, while pruning `apps/example` and `apps/test` from the search
- if exactly one valid existing entry file is found, reuse it
- if multiple valid existing entry files are found, ask the user to select one through an interactive prompt
- if no valid existing entry file is found, create `user/user_main.c` inside the new feature directory; never create it under `example` or `test`
- references read from `example` may explain API usage but must never become the selected writable entry file
- preserve the selected entry file's `user_main` function signature exactly
- do not rename `user_main` to `main`, `app_main`, `task_main`, or another name
- keep feature implementation out of the existing entry file
- modify the existing entry file only as needed to include the new feature header, declare integration symbols, initialize the feature, or call the feature API
- place helper functions and feature implementation in the new feature directory, not in the existing entry file
- preserve unrelated existing entry logic and required platform startup code
- do not add test cases, test loops, self-test routines, sample payloads, mock data, temporary diagnostics, or demo-only branches to `user_main.c`
- do not copy example test code into `user_main.c`
- production error handling and concise operational logging are allowed when required by the feature

## README Rules

The new feature directory must contain:

```text
<project>/apps/<new_feature_directory>/README.md
```

Rules:

- document the requested feature and application directory
- document `user/user_main.c` and the `user_main` entry function
- document additional generated files and their roles
- document the SDK interfaces used and the relevant local examples consulted
- document configuration assumptions and integration constraints
- document both supported config command forms and the required `build.bat all` compile command
- keep the README concise and relevant to the generated application

## C Coding Rules

Generated code must follow the verified SDK and nearby application conventions.

Rules:

- use only verified headers, APIs, macros, types, and signatures
- map every generated public behavior to an explicit user requirement; do not add unrelated capabilities
- implement each behavior through one clear path and avoid duplicated API, AT, callback, or state-handling logic
- follow nearby formatting, naming, configuration, and error-handling patterns
- keep functions short, focused, and named by purpose
- make initialization, normal operation, error handling, cleanup, and ownership boundaries easy to follow
- check relevant return values and handle failures consistently with verified SDK patterns
- prefer straightforward control flow over unnecessary wrappers, layers, flags, or abstractions
- use comments only when they clarify a non-obvious decision; do not narrate self-explanatory code
- prefer file-local static helpers where appropriate
- remove unused imports, variables, helpers, and copied example logic
- avoid placeholder implementations when working code was requested
- avoid changing SDK code, protected directories, or unrelated applications
- do not create or modify code outside the new feature directory, except for the single validated existing entry file

## Build Stage Interaction Rules

After every completed code-generation task, process the config and build stages separately. Each stage must have its own interactive VS Code confirmation dialog.

General rules:

- do not run build commands before code generation and consistency checks are complete
- call `vscode_askQuestions` directly for every stage confirmation; do not replace a required dialog with plain chat text
- do not combine config, build, and download confirmation into one question
- do not send the final generation response before all eligible stage dialogs have been answered
- run all commands from `<project>`
- run all commands through Windows `cmd`

Config-stage rules:

- immediately after generation, always show one question with `Run Config` and `Skip Config` buttons
- if the user selects `Skip Config`, do not run `build.bat config` and continue to the separate build-stage confirmation
- if the user selects `Run Config`, make one `vscode_askQuestions` call containing exactly two questions: target chip and firmware version
- both parameter questions must allow freeform input and must each provide a `Skip Parameters` option
- selecting `Skip Parameters` means use the build script's defaults; it does not insert literal default text into the command
- show `STM32F413RGT6` as the target-chip example and `your_firmware_version` as the firmware-version example
- if either answer is `Skip Parameters`, either input is empty, or no values are submitted, treat both parameters as omitted and run `build.bat config` without trailing parameters
- if the user supplies both values, append them in this order: target chip, then firmware version
- never run a one-parameter config command
- reject command-injection values containing shell control characters
- if the config command fails, report the configuration or environment error and do not show or run later stages

Build-stage rules:

- after config succeeds or the user explicitly selects `Skip Config`, always show a new question with `Build All` and `Skip Build All` buttons
- the build-stage dialog is mandatory and separate from the config-stage dialog
- if the user selects `Skip Build All`, do not run `build.bat all`, do not offer download, and finish after reporting the skipped stages
- if the user selects `Build All`, run `build.bat all`
- only a successful `build.bat all` result makes the download stage eligible

Parameterized command:

```text
cmd.exe /c "build.bat config <target_chip> <firmware_version>"
```

Example:

```text
cmd.exe /c "build.bat config STM32F413RGT6 your_firmware_version"
```

Skipped-parameter command:

```text
cmd.exe /c "build.bat config"
```

Required compile command after successful configuration:

```text
cmd.exe /c "build.bat all"
```

## Compile Failure Repair Rules

If `build.bat all` fails because of generated code, repair the generated code and execute `build.bat all` again automatically.

Rules:

- use compiler and linker output as the primary diagnostic source
- distinguish generated-code failures from missing tools, invalid parameters, environment failures, and unrelated pre-existing failures
- only generated-code repairs count toward the limit
- do not rerun `build.bat config` after each code repair when configuration already succeeded
- after each repair, rerun `build.bat all`
- allow at most 5 generated-code repair attempts
- do not show another build-stage dialog between automatic repair attempts
- stop immediately when compilation succeeds
- if compilation still fails after the fifth repair, stop, do not offer flashing, and report the concrete remaining errors, affected files, and attempted repairs
- repair only code inside `<project>/apps/<new_feature_directory>/` or the single validated existing entry file
- do not modify SDK code, build scripts, code outside `apps`, protected directories, or unrelated applications to force a successful build
- if the compiler error requires a code change outside the writable scope, report it as an external blocker and stop the repair loop

## Flash Rules

After every successful `build.bat all`, show a separate interactive VS Code dialog asking whether to execute `build.bat download`.

Rules:

- call `vscode_askQuestions` with explicit `Download` and `Skip Download` buttons
- the download-stage dialog is mandatory after every successful build and must not be merged with the config or build dialog
- do not show the download-stage dialog after skipped or failed compilation
- do not flash automatically
- run device detection and flashing from `<project>` through Windows `cmd`
- if the user selects `Skip Download`, finish without running device detection or download
- if the user selects `Download`, run the exact ST-Link detection command below

Required detection command:

```text
wmic path Win32_PnPEntity where "(PNPDeviceID like 'USB%%') and (Caption like '%%ST%%') and (Caption like '%%Link%%')" get Caption,DeviceID,Status
```

Detection rules:

- consider the device available only when the command returns a real matching device row containing both `ST` and `Link`
- do not treat the output header, `No Instance(s) Available`, an empty result, or a command error as a detected device
- if a matching device is present, run `cmd.exe /c "build.bat download"` from `<project>`
- if `wmic` is unavailable, report that device verification could not run; do not flash without verification
- if no matching device is present, do not run the download command
- send the official ST-LINK USB driver download page to the user: `https://www.st.com.cn/zh/development-tools/stsw-link009.html`
- tell the user to install the driver, connect the device, and reply exactly `继续烧录`
- before pausing for driver installation, retain the selected project path, the successful compile result, and a `waiting_for_flash_continuation` state in session context
- when the user later replies `继续烧录`, resume this workflow at ST-Link detection for the same project without regenerating or recompiling
- accept `继续烧录` as a resume command only when the retained state shows that compilation succeeded and flashing is awaiting continuation
- if the retained continuation state is unavailable or compilation was not successful, do not run download; explain that the project must be selected and compiled successfully first
- after resumed detection succeeds, run `build.bat download`
- after a successful download, clear the retained flash-continuation state
- if the download command fails, report the exact flash error and do not claim success
- finish only when flashing succeeds or the user selects `Skip Download`

## Response Contract

For code-generation requests, the agent must:

- acknowledge the request
- create a TODO plan
- execute immediately
- locate and validate the existing project layout
- search `quectel` for relevant SDK interface code first
- search `apps/example` for usage of those verified interfaces second
- use only the unified SDK API when it covers the requested behavior; do not add a duplicate AT implementation or fallback
- generate one new feature directory under `apps`
- keep every newly created code file inside that new feature directory
- locate and reuse one valid existing `user_main.c` under `apps` when available, excluding `example` and `test`
- create `user/user_main.c` in the new feature directory only when no valid existing entry file exists
- generate every non-entry code file inside the new feature directory
- never write to or modify `apps/example` or `apps/test`
- limit changes to a reused entry file to integration code for the new feature
- keep `user_main.c` free of tests, sample data, mocks, self-tests, and demo-only logic
- ensure generated behavior maps directly to the requirement and uses concise, clear control flow
- generate an application-level `README.md`
- always show the separate `Run Config` or `Skip Config` dialog after generation
- when config is selected, show both config parameter inputs and `Skip Parameters` choices in one interactive dialog
- run the parameterless config command when either input is empty or the user selects `Skip Parameters`
- after config succeeds or is explicitly skipped, always show the separate `Build All` or `Skip Build All` dialog
- after `build.bat all` succeeds, always show the separate `Download` or `Skip Download` dialog
- automatically repair generated-code compile failures up to 5 times
- verify an ST-Link device before running `build.bat download`
- provide the official driver URL and support `继续烧录` continuation when no device is found
- finish only after all TODO items and selected build and flash actions are complete

Do not end immediately after planning or research.

## Final Self-Check

Before finalizing, verify all of the following:

- one valid project with root-level `build.bat`, `quectel`, and `apps/example` was selected
- SDK interfaces were identified from `quectel` before example lookup
- usage examples were searched under `apps/example` using verified SDK symbols
- every requested operation was checked for a unified SDK API before considering AT commands
- no operation implemented by a unified SDK API also has an AT-command path or AT fallback
- exactly one new feature directory was created directly under `apps`
- every newly generated code file is under `<project>/apps/<new_feature_directory>/`
- the only permitted modified code file outside the new feature directory is the validated existing entry file
- no code file outside `apps` was created, modified, moved, renamed, or deleted
- the selected or newly created `user_main.c` is not under `apps/example` or `apps/test`
- an existing valid entry file was reused when available; a new entry file was created in the new feature directory only when none existed
- `user_main` retains the required entry name and existing signature
- `user_main.c` contains production integration only and no test, sample, mock, self-test, temporary diagnostic, or demo-only logic
- required feature code was added and unused example code was removed
- the new feature directory contains `README.md`
- no generated code was written into `quectel` or `apps/example`
- no file or directory under `apps/example` or `apps/test` was created, modified, moved, renamed, or deleted
- every generated non-entry code file was placed inside the new feature directory
- the final changed-file set contains no path under `apps/example` or `apps/test`
- only verified headers, APIs, macros, and types were used
- every generated behavior corresponds to the user requirement, with no unrelated capability
- control flow, naming, error handling, lifecycle, and ownership are concise and easy to follow
- duplicated business logic and unnecessary abstractions were not introduced
- the config-stage confirmation was shown immediately after generation and before the final response
- the config-stage confirmation used separate `Run Config` and `Skip Config` buttons
- both config inputs and `Skip Parameters` choices were presented through one interactive parameter dialog when config was selected
- the build-stage confirmation used separate `Build All` and `Skip Build All` buttons
- the config and build confirmations were separate dialogs
- the command ran from the project root through Windows `cmd`
- both build parameters or neither parameter were passed
- `build.bat all` ran only when the user selected `Build All`
- only a successful `build.bat all` result was treated as successful compilation
- `build.bat all` was rerun after every automatic code repair
- no more than 5 generated-code repair attempts were made
- the download-stage confirmation used separate `Download` and `Skip Download` buttons
- the download-stage confirmation was separate and appeared after every successful compilation
- ST-Link detection ran before download
- `build.bat download` ran only after a matching ST-Link device was detected
- missing-driver guidance included the official ST-LINK driver URL and `继续烧录` continuation instruction
- a paused flash flow retained the selected project and successful compile state for safe continuation
- successful flashing cleared the retained continuation state
- all TODO items are complete