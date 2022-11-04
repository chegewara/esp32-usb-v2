from esp_docs.conf_docs import *  # noqa: F403,F401

extensions += ['sphinx_copybutton',
               'sphinxcontrib.wavedrom',
               'esp_docs.esp_extensions.dummy_build_system',
               'esp_docs.esp_extensions.run_doxygen',
               ]
# link roles config
github_repo = 'chegewara/esp32-usb-v2'

# # context used by sphinx_idf_theme
html_context['github_user'] = 'chegewara'
html_context['github_repo'] = 'esp32-usb-v2'

html_static_path = ['../_static']

# # Extra options required by sphinx_idf_theme
# project_slug = 'caplamp'
# project_slug = 'esp-idf'
# versions_url = './_static/doc_versions.js'

idf_targets = ['esp32s2', 'esp32s3']
languages = ['en']
linkcheck_anchors = False
todo_include_todos = True

