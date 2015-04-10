<?php
/**
 * @file
 * Page callbacks accessible for users with administrative role only.
 */

/**
 * Module settings form.
 */
function commerce_collector_settings_form($form, &$form_state) {
  $form['commerce_collector_login'] = array(
    '#type' => 'fieldset',
    '#title' => t('Collector credentials'),
  );
  $form['commerce_collector_login']['commerce_collector_environment'] = array(
    '#type' => 'select',
    '#title' => t('Environment'),
    '#default_value' => variable_get('commerce_collector_environment', 'https://ecommercetest.collector.se/v3.0/InvoiceServiceV32.svc?singleWsdl'),
    '#options' => array(
      'https://ecommercetest.collector.se/v3.0/' => t('Test'),
      'https://ecommerce.collector.se/v3.0/' => t('Production'),
    ),
    '#description' => t('Choose which environment you want to use.'),
  );
  $form['commerce_collector_login']['commerce_collector_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('commerce_collector_username', NULL),
    '#description' => t('You should get this data from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('commerce_collector_password', NULL),
    '#description' => t('You should get this data from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_agreement_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Absolute path to collector agreement.'),
    '#default_value' => variable_get('commerce_collector_agreement_path', NULL),
    '#description' => t('If this field is not empty, required "agree to terms & conditions" checkbox will be added to payment form.'),
  );

  $form['commerce_collector_invoice'] = array(
    '#type' => 'fieldset',
    '#title' => t('Invoice settings'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_country'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Available countries'),
    '#required' => TRUE,
    '#multiple' => TRUE,
    '#default_value' => variable_get('commerce_collector_invoice_country', array()),
    '#options' => _commerce_collector_get_countries(),
    '#description' => t('This list has been made based on countries supported by Collector. Choose countries which are supported by your shop.'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_activation'] = array(
    '#type' => 'select',
    '#title' => t('Activation'),
    '#default_value' => variable_get('commerce_collector_invoice_activation', 0),
    '#options' => array(
      0 => t('Manual activation'),
    ),
    '#description' => t('Manual activation - Purchase will be preliminary and must be activated manually under Order Payment tab or through the Collector Partner Portal. You can also choose order states which will auto activate or cancel/credit payment for you.'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_activation_state'] = array(
    '#type' => 'select',
    '#title' => t('Auto-activate'),
    '#description' => t('Auto-activate invoice for one of selected states'),
    '#multiple' => TRUE,
    '#default_value' => variable_get('commerce_collector_invoice_activation_state', array('completed')),
    '#options' => commerce_order_state_options_list(),
    '#states' => array(
      'visible' => array(
        ':input[name="commerce_collector_invoice_activation"]' => array('value' => 0),
      ),
    ),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_deactivation_state'] = array(
    '#type' => 'select',
    '#title' => t('Auto-deactivate'),
    '#description' => t('Auto-deactivate invoice for one of selected states'),
    '#multiple' => TRUE,
    '#default_value' => variable_get('commerce_collector_invoice_deactivation_state', array('canceled')),
    '#options' => commerce_order_state_options_list(),
    '#states' => array(
      'visible' => array(
        ':input[name="commerce_collector_invoice_activation"]' => array('value' => 0),
      ),
    ),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_delivery'] = array(
    '#type' => 'select',
    '#title' => t('Delivery method'),
    '#default_value' => variable_get('commerce_collector_invoice_delivery', 2),
    '#options' => array(
      1 => t('Normal mail or in package'),
      2 => t('E-mail'),
    ),
    '#description' => t('Print and send invoice in package or allow Collector to send it for you (if e-mail address has been provided).'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_type'] = array(
    '#type' => 'select',
    '#title' => t('Invoice type'),
    '#default_value' => variable_get('commerce_collector_invoice_type', 0),
    '#options' => array(
      0 => t('Invoice in package or directly sent with e-mail'),
      1 => t('Monthly Invoice'),
      3 => t('Aggregated Invoice'),
      4 => t('Interest Free Invoice'),
      5 => t('Annuity invoice'),
    ),
    '#description' => t('Select one of available invoice types. By default - invoice per order.'),
  );

  $form['commerce_collector_vat'] = array(
    '#type' => 'fieldset',
    '#title' => t('VAT settings'),
  );
  $form['commerce_collector_vat']['commerce_collector_use_vat_module'] = array(
    '#type' => 'select',
    '#title' => t('Use VAT module'),
    '#description' => t('If VAT module has been enabled you might want to use it instead of fixed VAT rate.'),
    '#default_value' => variable_get('commerce_collector_use_vat_module', TRUE),
    '#options' => array(
      TRUE => t('VAT module'),
      FALSE => t('Fixed VAT rate'),
    ),
  );
  $form['commerce_collector_vat']['commerce_collector_fixed_vat_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed VAT rate'),
    '#description' => t('ex. "22" for 22% or "15.5" for 15,5%'),
    '#size' => 6,
    '#maxlength' => 6,
    '#default_value' => variable_get('commerce_collector_fixed_vat_rate', 0),
    '#element_validate' => array('_commerce_collector_validate_decimal_number'),
    '#states' => array(
      'visible' => array(
        ':input[name="commerce_collector_use_vat_module"]' => array('value' => 0),
      ),
    ),
  );
  return system_settings_form($form);
}

/**
 * Page callback to retrieve invoice via Collector API.
 */
function commerce_collector_admin_get_invoice($order, $transaction) {
  $data = array(
    'CountryCode' => $transaction->data['pane_values']['country'],
    'InvoiceNo' => $transaction->remote_id,
  );
  try {
    $result = _commerce_collector_request($data, 'GetCurrentInvoice', 'InformationService.svc', 'InformationService');
    $transaction->data['invoice'] = (array) $result;
    commerce_payment_transaction_save($transaction);
    drupal_set_message(t('Information about corresponding invoice has been updated.'));
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
  }
  // @todo: Use entity_uri.
  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment/' . $transaction->transaction_id);
}

/**
 * Wrapper over commerce_collector_activate_invoice() function.
 */
function commerce_collector_admin_activate_invoice($order, $transaction) {
  commerce_collector_activate_invoice($transaction);
  // @todo: Use entity_uri.
  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment/' . $transaction->transaction_id);
}

/**
 * Wrapper over commerce_collector_deactivate_invoice() function.
 */
function commerce_collector_admin_deactivate_invoice($order, $transaction) {
  commerce_collector_deactivate_invoice($transaction);
  // @todo: Use entity_uri.
  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment/' . $transaction->transaction_id);
}

/**
 * Validate decimal number.
 */
function _commerce_collector_validate_decimal_number($element, &$form_state) {
  $value = $element['#value'];
  $regexp = '@([^-0-9\\.])|(.-)@';
  if ($value != preg_replace($regexp, '', $value)) {
    form_error($element, t('Invalid number format. Should be decimal with . separator'));
  }
}
