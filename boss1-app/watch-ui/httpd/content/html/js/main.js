/**
 * @file main.js
 * @brief Main JavaScript for captive portal
 * @author BOSS1
 * @version 1.0
 * @date 2024
 */

(function() {
    'use strict';

    /* ========================================
       1. API Module
       ======================================== */
    window.WatchUI = window.WatchUI || {};

    WatchUI.api = {
        baseUrl: '/cgi-bin/cgi',

        async request(endpoint, data) {
            const url = this.baseUrl + endpoint;
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data || {}),
            });
            return response.json();
        },

        async login(username, password) {
            return this.request('/login', { username, password });
        },

        async getStatus() {
            return this.request('/status', {});
        },

        async logout() {
            return this.request('/logout', {});
        }
    };

    /* ========================================
       2. Utilities
       ======================================== */
    WatchUI.utils = {
        showError(message) {
            const errorEl = document.getElementById('error-message');
            if (errorEl) {
                errorEl.textContent = message;
                errorEl.style.display = 'block';
            }
        },

        hideError() {
            const errorEl = document.getElementById('error-message');
            if (errorEl) {
                errorEl.style.display = 'none';
            }
        },

        showLoading() {
            const btn = document.querySelector('button[type="submit"]');
            if (btn) {
                btn.disabled = true;
                btn.dataset.originalText = btn.textContent;
                btn.textContent = 'Loading...';
            }
        },

        hideLoading() {
            const btn = document.querySelector('button[type="submit"]');
            if (btn && btn.dataset.originalText) {
                btn.disabled = false;
                btn.textContent = btn.dataset.originalText;
            }
        }
    };

    /* ========================================
       3. Helper Functions
       ======================================== */
    function expandAccordion(element) {
        if (element && element.classList.contains('accordion-card')) {
            const content = element.querySelector('.accordion-content');
            const header = element.querySelector('.accordion-header');
            if (content && header && !content.classList.contains('expanded')) {
                content.classList.add('expanded');
                header.classList.add('expanded');
            }
        }
    }

    function scrollToSection(targetId) {
        const targetEl = document.getElementById(targetId);
        if (targetEl) {
            targetEl.scrollIntoView({ behavior: 'smooth' });
            setTimeout(function() {
                expandAccordion(targetEl);
            }, 300);
        }
    }

    /* ========================================
       4. Initialize Mobile TOC Panel
       ======================================== */
    function initMobileTocPanel() {
        const tocPanel = document.getElementById('toc-panel');
        if (tocPanel) return;

        const panel = document.createElement('div');
        panel.id = 'toc-panel';
        panel.innerHTML =
            '<div class="toc-header">' +
                '<span>目录</span>' +
                '<button class="toc-close" id="toc-close">' +
                    '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">' +
                        '<path d="M18 6L6 18M6 6l12 12"/>' +
                    '</svg>' +
                '</button>' +
            '</div>' +
            '<div style="flex:1;overflow-y:auto;padding:8px;">' +
                '<a href="#intro" class="toc-item">介绍</a>' +
                '<a href="#auth" class="toc-item">认证</a>' +
                '<a href="#help" class="toc-item">使用帮助</a>' +
                '<a href="#about" class="toc-item">关于我</a>' +
                '<a href="#statement" class="toc-item">声明</a>' +
            '</div>';
        document.body.appendChild(panel);

        const menuToggle = document.getElementById('menu-toggle');
        const closeBtn = document.getElementById('toc-close');

        if (menuToggle) {
            menuToggle.addEventListener('click', function(e) {
                e.preventDefault();
                panel.classList.add('active');
            });
        }

        if (closeBtn) {
            closeBtn.addEventListener('click', function(e) {
                e.preventDefault();
                panel.classList.remove('active');
            });
        }

        panel.addEventListener('click', function(e) {
            if (e.target === panel) {
                panel.classList.remove('active');
            }
        });

        panel.querySelectorAll('.toc-item').forEach(function(item) {
            item.addEventListener('click', function(e) {
                e.preventDefault();
                const href = this.getAttribute('href');
                if (href && href.startsWith('#')) {
                    const targetId = href.substring(1);
                    scrollToSection(targetId);
                }
                panel.classList.remove('active');
            });
        });
    }

    /* ========================================
       5. Theme Toggle
       ======================================== */
    function initThemeToggle() {
        const toggle = document.getElementById('theme-toggle');
        const html = document.documentElement;

        if (!toggle) return;

        const savedTheme = localStorage.getItem('theme') || 'light';
        html.setAttribute('data-theme', savedTheme);

        toggle.addEventListener('click', function() {
            const current = html.getAttribute('data-theme');
            const next = current === 'light' ? 'dark' : 'light';
            html.setAttribute('data-theme', next);
            localStorage.setItem('theme', next);
        });
    }

    /* ========================================
       6. Accordion Components
       ======================================== */
    function initAccordion(toggleId, contentId) {
        const toggle = document.getElementById(toggleId);
        const content = document.getElementById(contentId);

        if (toggle && content) {
            toggle.addEventListener('click', function() {
                content.classList.toggle('expanded');
                toggle.classList.toggle('expanded');
            });
        }
    }

    /* ========================================
       7. TOC Item Clicks (Desktop)
       ======================================== */
    function initTocLinks() {
        const tocItems = document.querySelectorAll('.toc-item');
        tocItems.forEach(function(item) {
            item.addEventListener('click', function(e) {
                const href = this.getAttribute('href');
                if (href && href.startsWith('#')) {
                    e.preventDefault();
                    const targetId = href.substring(1);
                    scrollToSection(targetId);
                }
            });
        });
    }

    /* ========================================
       8. Auth Form Validation
       ======================================== */
    function validateAuthForm(username, password) {
        // 账号验证：3-20位，只允许字母、数字、下划线
        const usernameRegex = /^[a-zA-Z0-9_]{3,20}$/;
        if (!username || !usernameRegex.test(username)) {
            return { valid: false, message: "账号只能包含字母、数字、下划线，长度3-20位" };
        }
        
        // 密码验证：6-20位，必须包含字母和数字
        const passwordRegex = /^(?=.*[a-zA-Z])(?=.*[0-9])[a-zA-Z0-9]{6,20}$/;
        if (!password || !passwordRegex.test(password)) {
            return { valid: false, message: "密码必须包含字母和数字，长度6-20位" };
        }
        
        return { valid: true };
    }

    /* ========================================
       9. Auth Form Handler
       ======================================== */
    function initAuthForm() {
        const form = document.getElementById('auth-form');
        const errorMsg = document.getElementById('error-msg');
        const successMsg = document.getElementById('success-msg');
        const submitBtn = document.getElementById('submit-btn');
        const usernameInput = document.getElementById('username');
        const passwordInput = document.getElementById('password');

        if (!form) return;

        form.addEventListener('submit', async function(e) {
            e.preventDefault();

            const username = usernameInput.value.trim();
            const password = passwordInput.value;

            // 客户端验证
            const validation = validateAuthForm(username, password);
            if (!validation.valid) {
                showError(validation.message);
                return;
            }

            submitBtn.disabled = true;
            submitBtn.textContent = '处理中...';
            hideMessages();

            try {
                const loginResp = await fetch('/cgi-bin/cgi/login', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ username, password })
                });

                const loginData = await loginResp.json();

                if (loginData.success) {
                    showSuccess('登录成功！欢迎回来，' + username);
                    return;
                }

                if (loginData.error === 'user_not_found' || loginData.error === 'invalid_password') {
                    const registerResp = await fetch('/cgi-bin/cgi/register', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ username, password })
                    });

                    const registerData = await registerResp.json();

                    if (registerData.success) {
                        showSuccess('注册成功！欢迎加入 BOSS1');
                    } else {
                        showError(registerData.error || '操作失败');
                    }
                } else {
                    showError(loginData.error || '操作失败');
                }
            } catch (err) {
                console.error('Auth error:', err);
                showSuccess('操作成功');
            }

            submitBtn.disabled = false;
            submitBtn.textContent = '进入';
        });

        function showError(msg) {
            errorMsg.textContent = msg;
            errorMsg.style.display = 'block';
            successMsg.style.display = 'none';
        }

        function showSuccess(msg) {
            successMsg.textContent = msg;
            successMsg.style.display = 'block';
            errorMsg.style.display = 'none';
        }

        function hideMessages() {
            errorMsg.style.display = 'none';
            successMsg.style.display = 'none';
        }
    }

    /* ========================================
       10. Flash Animation
       ======================================== */
    function initFlashAnimation() {
        window.addEventListener('load', function() {
            setTimeout(function() {
                const flash = document.getElementById('flash');
                if (flash) {
                    flash.classList.add('hide');
                }
            }, 2500);
        });
    }

    /* ========================================
       11. Initialize All Modules
       ======================================== */
    document.addEventListener('DOMContentLoaded', function() {
        initFlashAnimation();
        initThemeToggle();
        initMobileTocPanel();
        initAccordion('help-toggle', 'help-content');
        initAccordion('about-toggle', 'about-content');
        initTocLinks();
        initAuthForm();
    });

})();
